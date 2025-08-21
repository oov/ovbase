#include <ovbase.h>

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

#include <stdatomic.h>
static _Atomic uint64_t g_global_hint = 0;

#include "../3rd/hashmap.c/hashmap.h"
#include "mem.h"
#include <ovthreads.h>

#ifndef __FILE_NAME__
char const *ov_find_file_name(char const *s) {
  char const *found = s;
  for (; *s != '\0'; ++s) {
    if (*s == '/' || *s == '\\') {
      found = s + 1;
    }
  }
  return found;
}
#endif

static void global_hint_init(void) {
#ifdef _WIN32
  LARGE_INTEGER f = {0}, c = {0};
  if (QueryPerformanceFrequency(&f) != 0 && QueryPerformanceCounter(&c) != 0) {
    atomic_store(&g_global_hint, (c.QuadPart * 1000000000) / f.QuadPart);
  } else {
    atomic_store(&g_global_hint, GetTickCount() + GetCurrentProcessId() + GetCurrentThreadId());
  }
#else
  struct timespec v = {0};
  timespec_get(&v, TIME_UTC);
  atomic_store(&g_global_hint, v.tv_sec * 1000000000 + v.tv_nsec);
#endif
}

uint64_t get_global_hint(void) { return ov_splitmix64(atomic_fetch_add(&g_global_hint, 0x9e3779b97f4a7c15)); }

void *ov_hm_realloc(void *const p, size_t const s, void *const udata) {
  (void)udata;
  return REALLOC(p, s);
}
void ov_hm_free(void *const p, void *const udata) {
  (void)udata;
  FREE(p);
}

// mem

#ifdef ALLOCATE_LOGGER
static mtx_t g_mem_mtx = {0};
static struct hashmap *g_allocated = NULL;
struct allocated_at {
  void const *const p;
  struct ov_filepos const filepos;
};

static uint64_t am_hash(void const *const item, uint64_t const seed0, uint64_t const seed1, void const *const udata) {
  (void)udata;
  struct allocated_at const *const aa = item;
  return hashmap_sip(&aa->p, sizeof(void *), seed0, seed1);
}
static int am_compare(void const *const a, void const *const b, void const *udata) {
  struct allocated_at const *const aa0 = a;
  struct allocated_at const *const aa1 = b;
  (void)udata;
  return (int)((char const *)aa1->p - (char const *)aa0->p);
}

static void allocate_logger_init(void) {
  mtx_init(&g_mem_mtx, mtx_plain);
  uint64_t hash = ov_splitmix64_next(get_global_hint());
  uint64_t const s0 = ov_splitmix64(hash);
  hash = ov_splitmix64_next(hash);
  uint64_t const s1 = ov_splitmix64(hash);
  g_allocated = hashmap_new_with_allocator(
      ov_hm_realloc, ov_hm_free, sizeof(struct allocated_at), 8, s0, s1, am_hash, am_compare, NULL, NULL);
  if (!g_allocated) {
    abort();
  }
}

static void allocate_logger_exit(void) {
  hashmap_free(g_allocated);
  g_allocated = NULL;
  mtx_destroy(&g_mem_mtx);
}

static bool allocated_put(void const *const p MEM_FILEPOS_PARAMS) {
  hashmap_set(g_allocated,
              &(struct allocated_at){
                  .p = p,
                  .filepos = *filepos,
              });
  return hashmap_oom(g_allocated);
}

static bool allocated_remove(void const *const p) {
  struct allocated_at const *const aa = hashmap_delete(g_allocated, &(struct allocated_at){.p = p});
  return aa == NULL;
}

static bool report_leaks_iterate(void const *const item, void *const udata) {
  size_t *const n = udata;
  ++*n;
  struct allocated_at const *const aa = item;
  ereport(emsg_i18nf(err_type_generic,
                     err_unexpected,
                     NULL,
                     "Leak found #%zu: %s:%ld %s()",
                     *n,
                     aa->filepos.file,
                     aa->filepos.line,
                     aa->filepos.func));
  return true;
}

static size_t report_leaks(void) {
  // Make dummy to scan without lock
  uint64_t hash = ov_splitmix64_next(get_global_hint());
  uint64_t const s0 = ov_splitmix64(hash);
  hash = ov_splitmix64_next(hash);
  uint64_t const s1 = ov_splitmix64(hash);
  struct hashmap *dummy = hashmap_new_with_allocator(
      ov_hm_realloc, ov_hm_free, sizeof(struct allocated_at), 8, s0, s1, am_hash, am_compare, NULL, NULL);
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

long mem_get_allocated_count(void) { return atomic_load(&g_allocated_count); }
static inline void allocated(void) { atomic_fetch_add(&g_allocated_count, 1); }
static inline void freed(void) { atomic_fetch_sub(&g_allocated_count, 1); }
static void report_allocated_count(void) {
  long const n = mem_get_allocated_count();
  if (!n) {
    return;
  }
  ereport(emsg_i18nf(err_type_generic, err_unexpected, NULL, "Not freed memory blocks: %ld", n));
}
#endif

#if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
void mem_log_allocated(void const *const p MEM_FILEPOS_PARAMS) {
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

void mem_log_free(void const *const p) {
#  ifdef ALLOCATE_LOGGER
  mtx_lock(&g_mem_mtx);
  allocated_remove(p);
  mtx_unlock(&g_mem_mtx);
#  else
  (void)p;
#  endif
#  ifdef LEAK_DETECTOR
  freed();
#  endif
}
#endif

bool mem_core_(void *const pp, size_t const sz MEM_FILEPOS_PARAMS) {
  if (sz == 0) {
    if (*(void **)pp == NULL) {
      return false;
    }
#ifdef LEAK_DETECTOR
    freed();
#endif
#ifdef ALLOCATE_LOGGER
    {
      mtx_lock(&g_mem_mtx);
      bool const found_double_free = allocated_remove(*(void **)pp);
      mtx_unlock(&g_mem_mtx);
      if (found_double_free) {
        ereport(error_add_i18n_(
            NULL, err_type_generic, err_unexpected, "double free detected." MEM_FILEPOS_VALUES_PASSTHRU));
      }
    }
#endif
    FREE(*(void **)pp);
    *(void **)pp = NULL;
    return true;
  }
  void *np = REALLOC(*(void **)pp, sz);
  if (!np) {
    return false;
  }
  if (*(void **)pp == NULL) {
#ifdef LEAK_DETECTOR
    allocated();
#endif
#ifdef ALLOCATE_LOGGER
    {
      mtx_lock(&g_mem_mtx);
      bool const failed_allocate = allocated_put(np MEM_FILEPOS_VALUES_PASSTHRU);
      mtx_unlock(&g_mem_mtx);
      if (failed_allocate) {
        ereport(error_add_i18n_(
            NULL, err_type_generic, err_unexpected, "failed to record allocated memory." MEM_FILEPOS_VALUES_PASSTHRU));
      }
    }
#endif
  } else {
#ifdef ALLOCATE_LOGGER
    mtx_lock(&g_mem_mtx);
    bool const found_double_free = allocated_remove(*(void **)pp);
    bool const failed_allocate = allocated_put(np MEM_FILEPOS_VALUES_PASSTHRU);
    mtx_unlock(&g_mem_mtx);
    if (found_double_free) {
      ereport(
          error_add_i18n_(NULL, err_type_generic, err_unexpected, "double free detected." MEM_FILEPOS_VALUES_PASSTHRU));
    }
    if (failed_allocate) {
      ereport(error_add_i18n_(
          NULL, err_type_generic, err_unexpected, "failed to record allocated memory." MEM_FILEPOS_VALUES_PASSTHRU));
    }
#endif
  }
  *(void **)pp = np;
  return true;
}

#include "error.h"

void ov_init(void) {
  global_hint_init();
#ifdef ALLOCATE_LOGGER
  allocate_logger_init();
#endif
}

void ov_exit(void) {
#ifdef ALLOCATE_LOGGER
  report_leaks();
#endif
#ifdef LEAK_DETECTOR
  report_allocated_count();
#endif
#ifdef ALLOCATE_LOGGER
  allocate_logger_exit();
#endif
}
