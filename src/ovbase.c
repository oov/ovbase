#include <ovbase.h>

#include <ovrand.h>
#include <ovthreads.h> // struct timespec, timespec_get, TIME_UTC

#include <stdlib.h>
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include <stdatomic.h>
static _Atomic uint64_t g_global_hint = 0;

#include "mem.h"

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

uint64_t ov_rand_get_global_hint(void) {
  return ov_rand_splitmix64(atomic_fetch_add(&g_global_hint, 0x9e3779b97f4a7c15));
}

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
