#include "ovtest.h"

#include "def.h"

#include <inttypes.h>

#ifndef STR_PH
#  define STR_PH "ls"
#endif

#ifndef STRCMP
#  define STRCMP wcscmp
#endif

#ifdef __GNUC__
#  if __has_warning("-Wpadded")
#    pragma GCC diagnostic ignored "-Wpadded"
#  endif
#endif // __GNUC__

static void test_atoi_strict(void) {
  static const struct test_data {
    CHAR_TYPE const *input;
    bool result;
    INT_TYPE output;
  } test_data[] = {
      {
          .input = STR("0"),
          .output = INT64_C(0),
          .result = true,
      },
      {
          .input = STR("1"),
          .output = INT64_C(1),
          .result = true,
      },
      {
          .input = STR("-1"),
          .output = INT64_C(-1),
          .result = true,
      },
      {
          .input = STR("9223372036854775807"),
          .output = INT64_MAX,
          .result = true,
      },
      {
          .input = STR("9223372036854775808"),
          .result = false,
      },
      {
          .input = STR("-9223372036854775808"),
          .output = INT64_MIN,
          .result = true,
      },
      {
          .input = STR("-9223372036854775809"),
          .result = false,
      },
      {
          .input = STR("0x0"),
          .result = false,
      },
      {
          .input = STR("hello"),
          .result = false,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->input);
    INT_TYPE r = 0;
    if (TEST_CHECK(FUNCNAME(atoi)(td->input, &r, true) == td->result)) {
      TEST_CHECK(r == td->output);
    }
  }
}

static void test_atou_strict(void) {
  static const struct test_data {
    CHAR_TYPE const *input;
    bool result;
    UINT_TYPE output;
  } test_data[] = {
      {
          .input = STR("0"),
          .output = UINT64_C(0),
          .result = true,
      },
      {
          .input = STR("18446744073709551615"),
          .output = UINT64_MAX,
          .result = true,
      },
      {
          .input = STR("18446744073709551616"),
          .result = false,
      },
      {
          .input = STR("-1"),
          .result = false,
      },
      {
          .input = STR("0x0"),
          .result = false,
      },
      {
          .input = STR("hello"),
          .result = false,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->input);
    UINT_TYPE r = 0;
    if (TEST_CHECK(FUNCNAME(atou)(td->input, &r, true) == td->result)) {
      TEST_CHECK(r == td->output);
    }
  }
}

static void test_itoa(void) {
  static const struct test_data {
    INT_TYPE input;
    CHAR_TYPE const *output;
  } test_data[] = {
      {
          .input = INT64_C(0),
          .output = STR("0"),
      },
      {
          .input = INT64_C(1),
          .output = STR("1"),
      },
      {
          .input = INT64_MAX,
          .output = STR("9223372036854775807"),
      },
      {
          .input = INT64_C(-1),
          .output = STR("-1"),
      },
      {
          .input = INT64_MIN,
          .output = STR("-9223372036854775808"),
      },
  };
  CHAR_TYPE buf[32] = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->output);
    CHAR_TYPE *r = FUNCNAME(itoa)(td->input, buf);
    if (TEST_CHECK(r != NULL)) {
      TEST_CHECK(STRCMP(r, td->output) == 0);
      TEST_MSG("expected %" STR_PH, td->output);
      TEST_MSG("got %" STR_PH, r);
    }
  }
}

static void test_utoa(void) {
  static const struct test_data {
    UINT_TYPE input;
    CHAR_TYPE const *output;
  } test_data[] = {
      {
          .input = UINT64_C(0),
          .output = STR("0"),
      },
      {
          .input = UINT64_MAX,
          .output = STR("18446744073709551615"),
      },
  };

  CHAR_TYPE buf[32] = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->output);
    CHAR_TYPE *r = FUNCNAME(utoa)(td->input, buf);
    if (TEST_CHECK(r != NULL)) {
      TEST_CHECK(STRCMP(r, td->output) == 0);
      TEST_MSG("expected %" STR_PH, td->output);
      TEST_MSG("got %" STR_PH, r);
    }
  }
}

static void test_ftoa(void) {
  static const struct test_data {
    FLOAT_TYPE input;
    CHAR_TYPE const *output;
    size_t frac_len;
  } test_data[] = {
      {
          .input = 0.,
          .output = STR("0.00"),
          .frac_len = 2,
      },
      {
          .input = 1.,
          .output = STR("1.00"),
          .frac_len = 2,
      },
      {
          .input = 1.2,
          .output = STR("1.20"),
          .frac_len = 2,
      },
      {
          .input = 1.23,
          .output = STR("1.23"),
          .frac_len = 2,
      },
      {
          .input = 1.234,
          .output = STR("1.23"),
          .frac_len = 2,
      },
      {
          .input = 1.235,
          .output = STR("1.24"),
          .frac_len = 2,
      },
      {
          .input = 12.345,
          .output = STR("12.35"),
          .frac_len = 2,
      },
      {
          .input = -0.,
          .output = STR("0.00"),
          .frac_len = 2,
      },
      {
          .input = -1.,
          .output = STR("-1.00"),
          .frac_len = 2,
      },
      {
          .input = -1.2,
          .output = STR("-1.20"),
          .frac_len = 2,
      },
      {
          .input = -1.23,
          .output = STR("-1.23"),
          .frac_len = 2,
      },
      {
          .input = -1.234,
          .output = STR("-1.23"),
          .frac_len = 2,
      },
      {
          .input = -1.235,
          .output = STR("-1.24"),
          .frac_len = 2,
      },
      {
          .input = -12.345,
          .output = STR("-12.35"),
          .frac_len = 2,
      },
      {
          .input = 3.456,
          .output = STR("3"),
          .frac_len = 0,
      },
      {
          .input = 4.567,
          .output = STR("5"),
          .frac_len = 0,
      },
      {
          .input = -3.456,
          .output = STR("-3"),
          .frac_len = 0,
      },
      {
          .input = -4.567,
          .output = STR("-5"),
          .frac_len = 0,
      },
  };

  CHAR_TYPE buf[BUFFER_SIZE_FLOAT] = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->output);
    CHAR_TYPE *r = FUNCNAME(ftoa)(td->input, td->frac_len, STR('.'), buf);
    if (TEST_CHECK(r != NULL)) {
      TEST_CHECK(STRCMP(r, td->output) == 0);
      TEST_MSG("expected %" STR_PH, td->output);
      TEST_MSG("got %" STR_PH, r);
    }
  }
}

TEST_LIST = {
    {"test_atoi_strict", test_atoi_strict},
    {"test_atou_strict", test_atou_strict},
    {"test_itoa", test_itoa},
    {"test_utoa", test_utoa},
    {"test_ftoa", test_ftoa},
    {NULL, NULL},
};
