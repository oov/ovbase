#include "ovbase.h"

#include <stdarg.h>
#include <stdlib.h> // realloc, free

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  define REALLOC(ptr, size) (realloc(ptr, size))
#  define FREE(ptr) (free(ptr))
#else
#  include <stdio.h> // fprintf
#  define REALLOC(ptr, size) (realloc(ptr, size))
#  define FREE(ptr) (free(ptr))
#endif

#include <stdatomic.h>
static _Atomic uint64_t g_global_hint = 0;

#include "ovthreads.h"

#if __STDC_VERSION__ < 201112L || defined(__STDC_NO_THREADS__)
#  if defined(IMPLEMENT_BASE_TIMESPEC_WIN32)
int timespec_get(struct timespec *ts, int base) {
  if (!ts)
    return 0;
  if (base == TIME_UTC) {
    ts->tv_sec = time(NULL);
    ts->tv_nsec = 0;
    FILETIME ft1601 = {0};
    GetSystemTimeAsFileTime(&ft1601);
    uint64_t const t =
        (ULARGE_INTEGER){
            .LowPart = ft1601.dwLowDateTime,
            .HighPart = ft1601.dwHighDateTime,
        }
            .QuadPart -
        UINT64_C(0x019DB1DED53E8000); // 1601-01-01 to 1970-01-01
    ts->tv_sec = (time_t)(t / 10000000);
    ts->tv_nsec = (long)((t % 10000000) * 100);
    return base;
  }
  return 0;
}
#  endif

#  define DISABLE_TLS
#  define DISABLE_CALL_ONCE
#  ifdef __GNUC__
#    pragma GCC diagnostic push
#    if __has_warning("-Wreserved-identifier")
#      pragma GCC diagnostic ignored "-Wreserved-identifier"
#    endif
#    if __has_warning("-Wreserved-id-macro")
#      pragma GCC diagnostic ignored "-Wreserved-id-macro"
#    endif
#    if __has_warning("-Wpadded")
#      pragma GCC diagnostic ignored "-Wpadded"
#    endif
#    include "../3rd/tinycthread/source/tinycthread.c"
#    pragma GCC diagnostic pop
#  else
#    include "../3rd/tinycthread/source/tinycthread.c"
#  endif // __GNUC__
#  undef DISABLE_TLS
#  undef DISABLE_CALL_ONCE

#endif // __STDC_VERSION__ < 201112L || defined(__STDC_NO_THREADS__)

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  if __has_warning("-Wreserved-macro-identifier")
#    pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#  endif
#  if __has_warning("-Wpadded")
#    pragma GCC diagnostic ignored "-Wpadded"
#  endif
#  if __has_warning("-Wcast-align")
#    pragma GCC diagnostic ignored "-Wcast-align"
#  endif
#  if __has_warning("-Wsign-conversion")
#    pragma GCC diagnostic ignored "-Wsign-conversion"
#  endif
#  if __has_warning("-Wextra-semi-stmt")
#    pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#  endif
#  if __has_warning("-Wimplicit-fallthrough")
#    pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#  endif
#  include "../3rd/hashmap.c/hashmap.c"
#  pragma GCC diagnostic pop
#else
#  include "../3rd/hashmap.c/hashmap.c"
#endif // __GNUC__

#ifndef __FILE_NAME__
char const *ovbase_find_file_name(char const *s) {
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
    atomic_store_explicit(&g_global_hint, (c.QuadPart * 1000000000) / f.QuadPart, memory_order_relaxed);
  } else {
    atomic_store_explicit(
        &g_global_hint, GetTickCount() + GetCurrentProcessId() + GetCurrentThreadId(), memory_order_relaxed);
  }
#else
  struct timespec v = {0};
  timespec_get(&v, TIME_UTC);
  atomic_store_explicit(&g_global_hint, v.tv_sec * 1000000000 + v.tv_nsec, memory_order_relaxed);
#endif
}

uint64_t get_global_hint(void) {
  return ovbase_splitmix64(atomic_fetch_add_explicit(&g_global_hint, 0x9e3779b97f4a7c15, memory_order_relaxed));
}

static void write_stderr(NATIVE_CHAR const *const str) {
#ifdef _WIN32
  // https://docs.microsoft.com/en-us/windows/console/writeconsole
  // WriteConsole fails if it is used with a standard handle that is redirected
  // to a file. If an application processes multilingual output that can be
  // redirected, determine whether the output handle is a console handle (one
  // method is to call the GetConsoleMode function and check whether it
  // succeeds). If the handle is a console handle, call WriteConsole. If the
  // handle is not a console handle, the output is redirected and you should
  // call WriteFile to perform the I/O. Be sure to prefix a Unicode plain text
  // file with a byte order mark. For more information, see Using Byte Order
  // Marks.
  HANDLE const h = GetStdHandle(STD_ERROR_HANDLE);
  DWORD const len = (DWORD)wcslen(str);
  if (GetConsoleMode(h, &(DWORD){0})) {
    WriteConsoleW(h, str, len, NULL, NULL);
  } else {
    DWORD const plen = (DWORD)WideCharToMultiByte(CP_UTF8, 0, str, (int)len, NULL, 0, NULL, NULL);
    if (plen) {
      void *const p = malloc(plen);
      if (WideCharToMultiByte(CP_UTF8, 0, str, (int)len, p, (int)plen, NULL, NULL)) {
        WriteFile(h, p, plen, NULL, NULL);
      }
      free(p);
    }
  }
#else
  fprintf(stderr, NSTR("%s") NEWLINE, str);
#endif
}

static void *ovbase_hm_malloc(size_t const s, void *const udata) {
  (void)udata;
  return REALLOC(NULL, s);
}
static void ovbase_hm_free(void *const p, void *const udata) {
  (void)udata;
  FREE(p);
}

// mem

#ifdef ALLOCATE_LOGGER
static mtx_t g_mem_mtx = {0};
static struct hashmap *g_allocated = NULL;
struct allocated_at {
  void const *const p;
  struct ovbase_filepos const filepos;
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
  mtx_init(&g_mem_mtx, mtx_recursive);
  uint64_t hash = ovbase_splitmix64_next(get_global_hint());
  uint64_t const s0 = ovbase_splitmix64(hash);
  hash = ovbase_splitmix64_next(hash);
  uint64_t const s1 = ovbase_splitmix64(hash);
  g_allocated = hashmap_new_with_allocator(
      ovbase_hm_malloc, ovbase_hm_free, sizeof(struct allocated_at), 8, s0, s1, am_hash, am_compare, NULL, NULL);
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
    ereportmsg(errg(err_unexpected), &native_unmanaged(NSTR("failed to record allocated memory.")));
  }
}

static void allocated_remove(void const *const p) {
  struct allocated_at *const aa = hashmap_delete(g_allocated, &(struct allocated_at){.p = p});
  if (aa == NULL) {
    ereportmsg(emsg(err_type_generic, err_unexpected, &native_unmanaged(NSTR("double free detected."))),
               &native_unmanaged(NSTR("allocate logger report")));
  }
}

static bool report_leaks_iterate(void const *const item, void *const udata) {
  size_t *const n = udata;
  ++*n;
  struct allocated_at const *const aa = item;
  NATIVE_CHAR buf[1024] = {0};
#  ifdef _WIN32
  wsprintfW(buf, NSTR("Leak #%u: %hs:%ld %hs()") NEWLINE, *n, aa->filepos.file, aa->filepos.line, aa->filepos.func);
#  else
  sprintf(buf, NSTR("Leak #%zu: %s:%ld %s()") NEWLINE, *n, aa->filepos.file, aa->filepos.line, aa->filepos.func);
#  endif
  ereportmsg(emsg(err_type_generic, err_unexpected, &native_unmanaged(buf)),
             &native_unmanaged(NSTR("memory leak found")));
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

long mem_get_allocated_count(void) { return atomic_load_explicit(&g_allocated_count, memory_order_relaxed); }
static inline void allocated(void) { atomic_fetch_add_explicit(&g_allocated_count, 1, memory_order_relaxed); }
static inline void freed(void) { atomic_fetch_sub_explicit(&g_allocated_count, 1, memory_order_relaxed); }
static void report_allocated_count(void) {
  long const n = mem_get_allocated_count();
  if (!n) {
    return;
  }
  NATIVE_CHAR buf[64] = {0};
#  ifdef _WIN32
  wsprintfW(buf, NSTR("Not freed memory blocks: %ld") NEWLINE, n);
#  else
  sprintf(buf, NSTR("Not freed memory blocks: %ld") NEWLINE, n);
#  endif
  ereportmsg(emsg(err_type_generic, err_unexpected, &native_unmanaged(buf)),
             &native_unmanaged(NSTR("memory leak found")));
}
#endif

static bool mem_core_(void *const pp, size_t const sz MEM_FILEPOS_PARAMS) {
  if (sz == 0) {
    if (*(void **)pp == NULL) {
      return false;
    }
    FREE(*(void **)pp);
#ifdef LEAK_DETECTOR
    freed();
#endif
#ifdef ALLOCATE_LOGGER
    mtx_lock(&g_mem_mtx);
    allocated_remove(*(void **)pp);
    mtx_unlock(&g_mem_mtx);
#endif
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

error mem_(void *const pp, size_t const n, size_t const item_size MEM_FILEPOS_PARAMS) {
  if (!pp || !item_size) {
    return errg(err_invalid_arugment);
  }
  if (!mem_core_(pp, n * item_size MEM_FILEPOS_VALUES_PASSTHRU)) {
    return errg(err_out_of_memory);
  }
  return eok();
}

error mem_free_(void *const pp MEM_FILEPOS_PARAMS) {
  if (!pp) {
    return errg(err_invalid_arugment);
  }
  mem_core_(pp, 0 MEM_FILEPOS_VALUES_PASSTHRU);
  return eok();
}

#include "array.inc.c"
#include "error.inc.c"
#include "hmap.inc.c"
#include "str.inc.c"
#include "wstr.inc.c"

bool ovbase_init(void) {
  global_hint_init();
  if (!error_init()) {
    return false;
  }
#ifdef ALLOCATE_LOGGER
  allocate_logger_init();
#endif
  return error_register_default_mapper();
}

void ovbase_exit(void) {
#ifdef ALLOCATE_LOGGER
  report_leaks();
#endif
#ifdef LEAK_DETECTOR
  report_allocated_count();
#endif
#ifdef ALLOCATE_LOGGER
  allocate_logger_exit();
#endif
  error_exit();
}
