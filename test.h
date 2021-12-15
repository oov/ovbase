#pragma once

#ifdef __GNUC__

#pragma GCC diagnostic push
#if __has_warning("-Wused-but-marked-unused")
#pragma GCC diagnostic ignored "-Wused-but-marked-unused"
#endif

#endif // __GNUC__

#include "base.h"
#include <stdio.h>

static inline void test_init_(void) {
  if (!base_init()) {
    printf("!! INITIALIZATION FAILED !!\n");
    abort();
  }
#ifdef TEST_MY_INIT
  TEST_MY_INIT;
#endif
}

static inline void test_fini_(void) {
#ifdef TEST_MY_FINI
  TEST_MY_FINI;
#endif
  base_exit();
  if (mem_get_allocated_count()) {
    printf("!! MEMORY LEAKED !!\n");
    abort();
  }
}
#define TEST_INIT test_init_()
#define TEST_FINI test_fini_()

#ifdef __GNUC__

#pragma GCC diagnostic push
#if __has_warning("-Wpadded")
#pragma GCC diagnostic ignored "-Wpadded"
#endif
#if __has_warning("-Wsign-conversion")
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#include "3rd/acutest.h"
#pragma GCC diagnostic pop

#else

#include "3rd/acutest.h"

#endif // __GNUC__

static int test_eis(error err,
                    int const type,
                    uint_least32_t const code,
                    char const *const file,
                    int const line,
                    char const *const fmt,
                    char const *const p1,
                    char const *const p2,
                    char const *const p3) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
  int r = acutest_check_((err == NULL && type == 0 && code == 0) || eis(err, type, code), file, line, fmt, p1, p2, p3);
#pragma GCC diagnostic pop
  if (!r) {
    if (type == 0 && code == 0) {
      TEST_MSG("expected NULL");
    } else {
      TEST_MSG("expected %02x:%08x", type, code);
    }
    if (err == NULL) {
      TEST_MSG("got      NULL");
    } else {
      TEST_MSG("got      %02x:%08x", err->type, err->code);
      struct NATIVE_STR s = {0};
      error e = error_to_string(err, &s);
      if (esucceeded(e)) {
#ifdef _WIN32
        TEST_MSG("%ls\n", s.ptr);
#else
        TEST_MSG("%s\n", s.ptr);
#endif
      }
      efree(&e);
      ereport(sfree(&s));
    }
  }
  return r;
}
#define TEST_EIS(err, type, code) (test_eis((err), (type), (code)) __FILE__, __LINE__, "TEST_EIS(%s, %s, %s)", #err, #type, #code))
#define TEST_EISG(err, code)                                                                                           \
  (test_eis((err), err_type_generic, (code), __FILE__, __LINE__, "TEST_EISG(%s, %s)", #err, #code, NULL))
#define TEST_SUCCEEDED(err) (test_eis((err), 0, 0, __FILE__, __LINE__, "TEST_SUCCEEDED(%s)", #err, NULL, NULL))

static inline bool test_eis_f(error err,
                              int const type,
                              uint_least32_t const code,
                              char const *const file,
                              int const line,
                              char const *const fmt,
                              char const *const p1,
                              char const *const p2,
                              char const *const p3) {
  bool r = test_eis(err, type, code, file, line, fmt, p1, p2, p3);
  efree(&err);
  return r;
}
#define TEST_EIS_F(err, type, code)                                                                                    \
  (test_eis_f((err), (type), (code), __FILE__, __LINE__, "TEST_EIS_F(%s, %s, %s)", #err, #type, #code))
#define TEST_EISG_F(err, code)                                                                                         \
  (test_eis_f((err), err_type_generic, (code), __FILE__, __LINE__, "TEST_EISG_F(%s, %s)", #err, #code, NULL))
#define TEST_SUCCEEDED_F(err) (test_eis_f((err), 0, 0, __FILE__, __LINE__, "TEST_SUCCEEDED_F(%s)", #err, NULL, NULL))

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__
