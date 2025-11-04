#pragma once

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include <ovbase.h>

#ifdef __GNUC__

#  pragma GCC diagnostic push
#  if __has_warning("-Wused-but-marked-unused")
#    pragma GCC diagnostic ignored "-Wused-but-marked-unused"
#  endif

#endif // __GNUC__

static inline void test_init_(void) {
  setlocale(LC_ALL, "");
  ov_init();
#ifdef TEST_MY_INIT
  TEST_MY_INIT;
#endif
}

static inline void test_fini_(void) {
#ifdef TEST_MY_FINI
  TEST_MY_FINI;
#endif
  ov_exit();
#ifdef LEAK_DETECTOR
  if (ov_mem_get_allocated_count()) {
    printf("!! MEMORY LEAKED !!\n");
    abort();
  }
#endif
}
#define TEST_INIT test_init_()
#define TEST_FINI test_fini_()

#ifdef __GNUC__

#  pragma GCC diagnostic push
#  if __has_warning("-Wsign-conversion")
#    pragma GCC diagnostic ignored "-Wsign-conversion"
#  endif
#  if __has_warning("-Wmissing-prototypes")
#    pragma GCC diagnostic ignored "-Wmissing-prototypes"
#  endif
#  if __has_warning("-Wimplicit-int-float-conversion")
#    pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"
#  endif
#  if __has_warning("-Wdisabled-macro-expansion")
#    pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#  endif
#  if __has_warning("-Wunused-parameter")
#    pragma GCC diagnostic ignored "-Wunused-parameter"
#  endif
#  if __has_warning("-Wunused-function")
#    pragma GCC diagnostic ignored "-Wunused-function"
#  endif
#  if __has_warning("-Wswitch-enum")
#    pragma GCC diagnostic ignored "-Wswitch-enum"
#  endif
#  if __has_warning("-Wswitch-default")
#    pragma GCC diagnostic ignored "-Wswitch-default"
#  endif
#  if __has_warning("-Wformat")
#    pragma GCC diagnostic ignored "-Wformat"
#  endif
#  include <ovbase_3rd/acutest.h>
#  pragma GCC diagnostic pop

#else

#  include <ovbase_3rd/acutest.h>

#endif // __GNUC__

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif // __GNUC__

static inline bool ovtest_should_run_benchmarks(void) {
  char const *env = getenv("OVTEST_RUN_BENCHMARKS");
  if (env && env[0] == '1' && env[1] == '\0') {
    return true;
  }
  TEST_SKIP("Skipping benchmark tests. Set OVTEST_RUN_BENCHMARKS=1 environment variable to run.");
  return false;
}
