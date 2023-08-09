#include "ovbase.h"

#include <stdlib.h> // realloc, free

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  define REALLOC(ptr, size) (realloc(ptr, size))
#  define FREE(ptr) (free(ptr))
#else
#  define REALLOC(ptr, size) (realloc(ptr, size))
#  define FREE(ptr) (free(ptr))
#endif

#include <stdatomic.h>
static _Atomic uint64_t g_global_hint = 0;

#include "../3rd/hashmap.c/hashmap.h"
#include "mem.h"
#include "ovthreads.h"

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

void *ov_hm_malloc(size_t const s, void *const udata) {
  (void)udata;
  return REALLOC(NULL, s);
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

static uint64_t am_hash(void const *const item, uint64_t const seed0, uint64_t const seed1, void *const udata) {
  (void)udata;
  struct allocated_at const *const aa = item;
  return hashmap_sip(&aa->p, sizeof(void *), seed0, seed1);
}
static int am_compare(void const *const a, void const *const b, void *udata) {
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
      ov_hm_malloc, ov_hm_free, sizeof(struct allocated_at), 8, s0, s1, am_hash, am_compare, NULL, NULL);
  if (!g_allocated) {
    abort();
  }
}

static void allocate_logger_exit(void) {
  hashmap_free(g_allocated);
  g_allocated = NULL;
  mtx_destroy(&g_mem_mtx);
}

static void allocated_put(void const *const p MEM_FILEPOS_PARAMS) {
  hashmap_set(g_allocated,
              &(struct allocated_at){
                  .p = p,
                  .filepos = *filepos,
              });
  if (hashmap_oom(g_allocated)) {
    ereport(emsg_i18nf(err_type_generic, err_unexpected, NULL, "%s", "failed to record allocated memory."));
  }
}

static void allocated_remove(void const *const p) {
  struct allocated_at *const aa = hashmap_delete(g_allocated, &(struct allocated_at){.p = p});
  if (aa == NULL) {
    ereport(emsg_i18nf(err_type_generic, err_unexpected, NULL, "%s", "double free detected."));
  }
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
  size_t n = 0;
  mtx_lock(&g_mem_mtx);
  hashmap_scan(g_allocated, report_leaks_iterate, &n);
  mtx_unlock(&g_mem_mtx);
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

bool mem_core_(void *const pp, size_t const sz MEM_FILEPOS_PARAMS) {
  if (sz == 0) {
    if (*(void **)pp == NULL) {
      return false;
    }
#ifdef LEAK_DETECTOR
    freed();
#endif
#ifdef ALLOCATE_LOGGER
    mtx_lock(&g_mem_mtx);
    allocated_remove(*(void **)pp);
    mtx_unlock(&g_mem_mtx);
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
    mtx_lock(&g_mem_mtx);
    allocated_put(np MEM_FILEPOS_VALUES_PASSTHRU);
    mtx_unlock(&g_mem_mtx);
#endif
  } else {
#ifdef ALLOCATE_LOGGER
    mtx_lock(&g_mem_mtx);
    allocated_remove(*(void **)pp);
    allocated_put(np MEM_FILEPOS_VALUES_PASSTHRU);
    mtx_unlock(&g_mem_mtx);
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
