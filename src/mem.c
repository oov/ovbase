#include "mem.h"

#include <assert.h>

#ifdef USE_MIMALLOC

#  ifdef __GNUC__
#    pragma GCC diagnostic push
#    if __has_warning("-Wreserved-identifier")
#      pragma GCC diagnostic ignored "-Wreserved-identifier"
#    endif
#  endif // __GNUC__
#  include <mimalloc.h>
#  ifdef __GNUC__
#    pragma GCC diagnostic pop
#  endif // __GNUC__

#  define REALLOC(ptr, size) (mi_realloc(ptr, size))
#  define FREE(ptr) (mi_free(ptr))

#else

#  define REALLOC(ptr, size) (realloc(ptr, size))
#  define FREE(ptr) (free(ptr))

#endif

#include <stdlib.h> // realloc, free
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include "../3rd/hashmap.c/hashmap.h"
#include <ovprintf.h>
#include <ovrand.h>
#include <ovthreads.h>
#include <stdatomic.h>

#include "output.h"

// mem

#ifdef ALLOCATE_LOGGER
static mtx_t g_mem_mtx = {0};
static struct hashmap *g_allocated = NULL;
struct allocated_at {
  void const *const p;
  struct ov_filepos const filepos;
};

static uint64_t am_hash(void const *const item, uint64_t const seed0, uint64_t const seed1, void const *const udata) {
  assert(item != NULL && "item must not be NULL");
  (void)udata;
  (void)seed1;
  struct allocated_at const *const aa = (struct allocated_at const *)item;
  return hashmap_xxhash3(&aa->p, sizeof(void *), seed0, seed1);
}
static int am_compare(void const *const a, void const *const b, void const *udata) {
  assert(a != NULL && "a must not be NULL");
  assert(b != NULL && "b must not be NULL");
  struct allocated_at const *const aa0 = (struct allocated_at const *)a;
  struct allocated_at const *const aa1 = (struct allocated_at const *)b;
  (void)udata;
  return (int)((char const *)aa1->p - (char const *)aa0->p);
}

static void *am_realloc(void *p, size_t const s, void *const udata) {
  (void)udata;
  return REALLOC(p, s);
}

static void am_free(void *p, void *const udata) {
  (void)udata;
  FREE(p);
}

void allocate_logger_init(void) {
  mtx_init(&g_mem_mtx, mtx_plain);
  uint64_t hash = ov_rand_splitmix64_next(ov_rand_get_global_hint());
  uint64_t const s0 = ov_rand_splitmix64(hash);
  hash = ov_rand_splitmix64_next(hash);
  uint64_t const s1 = ov_rand_splitmix64(hash);
  g_allocated = hashmap_new_with_allocator(
      am_realloc, am_free, sizeof(struct allocated_at), 8, s0, s1, am_hash, am_compare, NULL, NULL);
  if (!g_allocated) {
    abort();
  }
}

void allocate_logger_exit(void) {
  hashmap_free(g_allocated);
  g_allocated = NULL;
  mtx_destroy(&g_mem_mtx);
}

static bool allocated_put(void const *const p MEM_FILEPOS_PARAMS) {
  assert(p != NULL && "p must not be NULL");
  assert(filepos != NULL && "filepos must not be NULL");
  hashmap_set(g_allocated,
              &(struct allocated_at){
                  .p = p,
                  .filepos = *filepos,
              });
  return hashmap_oom(g_allocated);
}

static bool allocated_remove(void const *const p) {
  assert(p != NULL && "p must not be NULL");
  struct allocated_at const *const aa =
      (struct allocated_at const *)hashmap_delete(g_allocated, &(struct allocated_at){.p = p});
  return aa == NULL;
}

static bool report_leaks_iterate(void const *const item, void *const udata) {
  assert(item != NULL && "item must not be NULL");
  assert(udata != NULL && "udata must not be NULL");
  size_t *const n = (size_t *)udata;
  ++*n;
  struct allocated_at const *const aa = (struct allocated_at const *)item;
  {
    char buffer[512];
    OV_SNPRINTF(buffer,
                sizeof(buffer),
                NULL,
                "Leak found #%zu: %s:%ld %s()\n",
                *n,
                aa->filepos.file,
                aa->filepos.line,
                aa->filepos.func);
    output(buffer);
  }
  return true;
}

size_t report_leaks(void) {
  // Make dummy to scan without lock
  uint64_t hash = ov_rand_splitmix64_next(ov_rand_get_global_hint());
  uint64_t const s0 = ov_rand_splitmix64(hash);
  hash = ov_rand_splitmix64_next(hash);
  uint64_t const s1 = ov_rand_splitmix64(hash);
  struct hashmap *dummy = hashmap_new_with_allocator(
      am_realloc, am_free, sizeof(struct allocated_at), 8, s0, s1, am_hash, am_compare, NULL, NULL);
  if (!dummy) {
    abort();
  }

  mtx_lock(&g_mem_mtx);
  {
    struct hashmap *tmp = g_allocated;
    g_allocated = dummy;
    dummy = tmp;
  }
  mtx_unlock(&g_mem_mtx);
  size_t n = 0;
  hashmap_scan(dummy, report_leaks_iterate, &n);
  hashmap_free(dummy);
  return n;
}

#endif

#ifdef LEAK_DETECTOR
static atomic_long g_allocated_count = 0;

long ov_mem_get_allocated_count(void) { return atomic_load(&g_allocated_count); }

static inline void allocated(void) { atomic_fetch_add(&g_allocated_count, 1); }
static inline void freed(void) { atomic_fetch_sub(&g_allocated_count, 1); }
void report_allocated_count(void) {
  long const n = ov_mem_get_allocated_count();
  if (!n) {
    return;
  }
  {
    char buffer[256];
    OV_SNPRINTF(buffer, sizeof(buffer), NULL, "Not freed memory blocks: %ld\n", n);
    output(buffer);
  }
}
#endif

#if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
#  ifdef ALLOCATE_LOGGER
static void report_error(char const *const message, struct ov_filepos const *const filepos) {
  assert(message != NULL && "message must not be NULL");
  assert(filepos != NULL && "filepos must not be NULL");
  char buffer[256];
  OV_SNPRINTF(
      buffer, sizeof(buffer), NULL, "%s at %s:%ld %s()\n", message, filepos->file, filepos->line, filepos->func);
  output(buffer);
}
#  endif

void mem_log_allocated(void const *const p MEM_FILEPOS_PARAMS) {
  assert(p != NULL && "p must not be NULL");
#  ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
#  endif
#  ifdef ALLOCATE_LOGGER
  mtx_lock(&g_mem_mtx);
  allocated_put(p MEM_FILEPOS_VALUES_PASSTHRU);
  mtx_unlock(&g_mem_mtx);
#  else
  (void)p;
#  endif
#  ifdef LEAK_DETECTOR
  allocated();
#  endif
}

void mem_log_free(void const *const p MEM_FILEPOS_PARAMS) {
  assert(p != NULL && "p must not be NULL");
#  ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
  mtx_lock(&g_mem_mtx);
  bool const found_double_free = allocated_remove(p);
  mtx_unlock(&g_mem_mtx);
  if (found_double_free) {
    report_error("double free detected", filepos);
  }
#  else
  (void)p;
#  endif
#  ifdef LEAK_DETECTOR
  freed();
#  endif
}

void mem_log_realloc_validate(void const *const old_p MEM_FILEPOS_PARAMS) {
#  ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
  if (old_p != NULL) {
    mtx_lock(&g_mem_mtx);
    bool const found_uninitialized = allocated_remove(old_p);
    mtx_unlock(&g_mem_mtx);
    if (found_uninitialized) {
      report_error("uninitialized or invalid pointer detected", filepos);
    }
  }
#  else
  (void)old_p;
#  endif
}

void mem_log_realloc_update(void const *const new_p MEM_FILEPOS_PARAMS) {
  assert(new_p != NULL && "new_p must not be NULL");
#  ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
  mtx_lock(&g_mem_mtx);
  bool const failed_allocate = allocated_put(new_p MEM_FILEPOS_VALUES_PASSTHRU);
  mtx_unlock(&g_mem_mtx);
  if (failed_allocate) {
    report_error("failed to record allocated memory", filepos);
  }
#  else
  (void)new_p;
#  endif
}
#endif

bool mem_core_(void *const pp, size_t const sz MEM_FILEPOS_PARAMS) {
  assert(pp != NULL && "pp must not be NULL");
#ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
#endif
  if (sz == 0) {
    if (*(void **)pp == NULL) {
      return false;
    }
#if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
    mem_log_free(*(void **)pp MEM_FILEPOS_VALUES_PASSTHRU);
#endif
    FREE(*(void **)pp);
    *(void **)pp = NULL;
    return true;
  }
#if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
  if (*(void **)pp != NULL) {
    mem_log_realloc_validate(*(void **)pp MEM_FILEPOS_VALUES_PASSTHRU);
  }
#endif
  void *np = REALLOC(*(void **)pp, sz);
  if (!np) {
    return false;
  }
#if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
  if (*(void **)pp == NULL) {
    mem_log_allocated(np MEM_FILEPOS_VALUES_PASSTHRU);
  } else {
    mem_log_realloc_update(np MEM_FILEPOS_VALUES_PASSTHRU);
  }
#endif
  *(void **)pp = np;
  return true;
}

bool ov_mem_realloc(void *const pp, size_t const n, size_t const item_size MEM_FILEPOS_PARAMS) {
  assert(pp != NULL && "pp must not be NULL");
  assert(n > 0 && "n must be greater than 0");
  assert(item_size > 0 && "item_size must be greater than 0");
#ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
#endif
  if (!pp || !n || !item_size) {
    return false;
  }
  if (!mem_core_(pp, n * item_size MEM_FILEPOS_VALUES_PASSTHRU)) {
    return false;
  }
  return true;
}

void ov_mem_free(void *const pp MEM_FILEPOS_PARAMS) {
  assert(pp != NULL && "pp must not be NULL");
#ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
#endif
  if (!pp) {
    return;
  }
  mem_core_(pp, 0 MEM_FILEPOS_VALUES_PASSTHRU);
  return;
}
