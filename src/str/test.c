#include <ovtest.h>

#include <inttypes.h>

#ifdef USE_STR

#  ifdef _WIN32
#    define STR_PH "hs"
#  else
#    define STR_PH "s"
#  endif

static void test_str_cpy_free(void) {
  static char const test_str[] = "hello";
  struct str s = {0};
  if (TEST_SUCCEEDED_F(scpy(&s, test_str))) {
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, test_str) == 0);
    }
    TEST_CHECK(s.len == strlen(test_str));
    TEST_CHECK(s.cap > strlen(test_str));
  }

  static char const test_str2[] = "good bye world";
  if (TEST_SUCCEEDED_F(scpy(&s, test_str2))) {
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, test_str2) == 0);
    }
    TEST_CHECK(s.len == strlen(test_str2));
    TEST_CHECK(s.cap > strlen(test_str2));
  }

  static char const test_str3[] = "";
  if (TEST_SUCCEEDED_F(scpy(&s, test_str3))) {
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, test_str3) == 0);
    }
    TEST_CHECK(s.len == strlen(test_str3));
    TEST_CHECK(s.cap > strlen(test_str3));
  }

  if (TEST_SUCCEEDED_F(sfree(&s))) {
    TEST_CHECK(s.ptr == NULL);
    TEST_CHECK(s.len == 0);
    TEST_CHECK(s.cap == 0);
  }

  TEST_EISG_F(scpy(&s, NULL), err_invalid_arugment);
}

static void test_str_cpym(void) {
  static char const s1[] = "hello";
  static char const s2[] = "world";
  static char const expected[] = "helloworld";
  struct str s = {0};
  if (TEST_SUCCEEDED_F(scpym(&s, s1, s2))) {
    TEST_CHECK(strcmp(s.ptr, expected) == 0);
    TEST_MSG("expected %" STR_PH, expected);
    TEST_MSG("got %" STR_PH, s.ptr);
  }
  if (TEST_SUCCEEDED_F(scpym(&s, s1, s2))) {
    TEST_CHECK(strcmp(s.ptr, expected) == 0);
    TEST_MSG("expected %" STR_PH, expected);
    TEST_MSG("got %" STR_PH, s.ptr);
  }
  TEST_SUCCEEDED_F(sfree(&s));
}

static void test_str_ncpy(void) {
  static char const test_str[] = "hello";
  struct str s = {0};
  if (TEST_SUCCEEDED_F(sncpy(&s, test_str, 2))) {
    TEST_CHECK(s.ptr != NULL);
    TEST_CHECK(s.len == 2);
    TEST_CHECK(s.cap > 2);
    TEST_CHECK(s.ptr[s.len] == L'\0');
    TEST_SUCCEEDED_F(sfree(&s));
  }
  if (TEST_SUCCEEDED_F(sncpy(&s, test_str, 5))) {
    if (TEST_SUCCEEDED_F(sncpy(&s, test_str, 2))) {
      TEST_CHECK(s.ptr != NULL);
      TEST_CHECK(s.len == 2);
      TEST_CHECK(s.cap > 2);
      TEST_CHECK(s.ptr[s.len] == L'\0');
      TEST_SUCCEEDED_F(sfree(&s));
    }
  }
  if (TEST_SUCCEEDED_F(sncpy(&s, test_str, 100))) {
    TEST_CHECK(s.ptr != NULL);
    TEST_CHECK(s.len == 5);
    TEST_CHECK(s.cap > 5);
    TEST_CHECK(s.ptr[s.len] == L'\0');
    TEST_SUCCEEDED_F(sfree(&s));
  }
}

static void test_str_cat_free(void) {
  static char const test_str[] = "hello";
  struct str s = {0};
  TEST_SUCCEEDED_F(scpy(&s, test_str));
  static char const test_str2[] = "world";
  if (TEST_SUCCEEDED_F(scat(&s, test_str2))) {
    static char const expected_str[] = "helloworld";
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, expected_str) == 0);
    }
    TEST_CHECK(s.len == strlen(expected_str));
    TEST_CHECK(s.cap > strlen(expected_str));
  }

  static char const test_str3[] = "";
  if (TEST_SUCCEEDED_F(scat(&s, test_str3))) {
    static char const expected_str[] = "helloworld";
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, expected_str) == 0);
    }
    TEST_CHECK(s.len == strlen(expected_str));
    TEST_CHECK(s.cap > strlen(expected_str));
  }

  if (TEST_SUCCEEDED_F(sfree(&s))) {
    TEST_CHECK(s.ptr == NULL);
    TEST_CHECK(s.len == 0);
    TEST_CHECK(s.cap == 0);
  }

  if (TEST_SUCCEEDED_F(scat(&s, test_str))) {
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, test_str) == 0);
    }
    TEST_CHECK(s.len == strlen(test_str));
    TEST_CHECK(s.cap > strlen(test_str));
  }
  TEST_SUCCEEDED_F(sfree(&s));
  TEST_EISG_F(scat(&s, NULL), err_invalid_arugment);
}

static void test_str_catm(void) {
  static char const s1[] = "hello";
  static char const s2[] = "world";
  static char const expected1[] = "helloworld";
  static char const expected2[] = "helloworldhelloworld";
  struct str s = {0};
  if (TEST_SUCCEEDED_F(scatm(&s, s1, s2))) {
    TEST_CHECK(strcmp(s.ptr, expected1) == 0);
    TEST_MSG("expected %" STR_PH, expected1);
    TEST_MSG("got %" STR_PH, s.ptr);
  }
  if (TEST_SUCCEEDED_F(scatm(&s, s1, s2))) {
    TEST_CHECK(strcmp(s.ptr, expected2) == 0);
    TEST_MSG("expected %" STR_PH, expected2);
    TEST_MSG("got %" STR_PH, s.ptr);
  }
  TEST_SUCCEEDED_F(sfree(&s));
}

static void test_str_ncat(void) {
  static char const test_str[] = "hello";
  static char const test2_str[] = "world";
  struct str s = {0};
  if (TEST_SUCCEEDED_F(sncat(&s, test_str, 2))) {
    TEST_CHECK(s.ptr != NULL);
    TEST_CHECK(s.len == 2);
    TEST_CHECK(s.cap > 2);
    TEST_CHECK(strcmp(s.ptr, "he") == 0);
    TEST_SUCCEEDED_F(sfree(&s));
  }
  if (TEST_SUCCEEDED_F(sncat(&s, test_str, 100))) {
    TEST_CHECK(s.ptr != NULL);
    TEST_CHECK(s.len == 5);
    TEST_CHECK(s.cap > 5);
    TEST_CHECK(s.ptr[s.len] == L'\0');
    TEST_CHECK(strcmp(s.ptr, "hello") == 0);
    TEST_SUCCEEDED_F(sfree(&s));
  }

  if (TEST_SUCCEEDED_F(sncat(&s, test_str, 2))) {
    if (TEST_SUCCEEDED_F(sncat(&s, test2_str, 2))) {
      TEST_CHECK(s.ptr != NULL);
      TEST_CHECK(s.len == 4);
      TEST_CHECK(s.cap > 4);
      TEST_CHECK(strcmp(s.ptr, "hewo") == 0);
    }
    TEST_SUCCEEDED_F(sfree(&s));
  }
  if (TEST_SUCCEEDED_F(sncat(&s, test_str, 2))) {
    if (TEST_SUCCEEDED_F(sncat(&s, test2_str, 100))) {
      TEST_CHECK(s.ptr != NULL);
      TEST_CHECK(s.len == 7);
      TEST_CHECK(s.cap > 7);
      TEST_CHECK(strcmp(s.ptr, "heworld") == 0);
    }
    TEST_SUCCEEDED_F(sfree(&s));
  }
}

static void test_str_grow(void) {
  struct str s = {0};
  if (TEST_SUCCEEDED_F(sgrow(&s, 1))) {
    TEST_CHECK(s.ptr != NULL);
    TEST_CHECK(s.len == 0);
    TEST_CHECK(s.cap >= 1);
  }
  TEST_SUCCEEDED_F(sfree(&s));
}

static void test_str_str(void) {
  struct str s = str_unmanaged("hello");
  ptrdiff_t pos = 0, expected = -1;
  if (TEST_SUCCEEDED_F(sstr(&s, "!", &pos))) {
    TEST_CHECK(pos == expected);
    TEST_MSG("expected %td", expected);
    TEST_MSG("got %td", pos);
  }
  expected = 1;
  if (TEST_SUCCEEDED_F(sstr(&s, "e", &pos))) {
    TEST_CHECK(pos == expected);
    TEST_MSG("expected %td", expected);
    TEST_MSG("got %td", pos);
  }
  expected = 4;
  if (TEST_SUCCEEDED_F(sstr(&s, "o", &pos))) {
    TEST_CHECK(pos == expected);
    TEST_MSG("expected %td", expected);
    TEST_MSG("got %td", pos);
  }
  TEST_EISG_F(sstr(&s, NULL, NULL), err_invalid_arugment);
  TEST_EISG_F(sstr(&s, "e", NULL), err_null_pointer);
}

static void test_str_replace_all(void) {
  struct str s = {0};
  if (TEST_SUCCEEDED_F(scpy(&s, "hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&s, "o", "xx"))) {
      static char const expected[] = "hellxx wxxrld";
      TEST_CHECK(strcmp(s.ptr, expected) == 0);
      TEST_MSG("expected %" STR_PH, expected);
      TEST_MSG("got %" STR_PH, s.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&s, "hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&s, "o", ""))) {
      static char const expected[] = "hell wrld";
      TEST_CHECK(strcmp(s.ptr, expected) == 0);
      TEST_MSG("expected %" STR_PH, expected);
      TEST_MSG("got %" STR_PH, s.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&s, "hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&s, "d", "xx"))) {
      static char const expected[] = "hello worlxx";
      TEST_CHECK(strcmp(s.ptr, expected) == 0);
      TEST_MSG("expected %" STR_PH, expected);
      TEST_MSG("got %" STR_PH, s.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&s, "hello world"))) {
    TEST_SUCCEEDED_F(sreplace_all(&s, "", "xx"));
    TEST_SUCCEEDED_F(sreplace_all(&s, "a", "xx"));
    TEST_EISG_F(sreplace_all(&s, NULL, NULL), err_invalid_arugment);
    TEST_EISG_F(sreplace_all(&s, "o", NULL), err_invalid_arugment);
    TEST_EISG_F(sreplace_all(&s, NULL, "xx"), err_invalid_arugment);
  }
  TEST_SUCCEEDED_F(sfree(&s));
}

static void test_str_const(void) {
  static char const s[] = "hello";
  ptrdiff_t pos = 0;
  if (TEST_SUCCEEDED_F(sstr(&str_unmanaged_const(s), "o", &pos))) {
    TEST_CHECK(pos == 4);
  }
}

static void test_str_atoi(void) {
  static const struct test_data {
    char const *input;
    int code;
    int64_t output;
  } test_data[] = {
      {
          .input = "0",
          .output = INT64_C(0),
          .code = 0,
      },
      {
          .input = "1",
          .output = INT64_C(1),
          .code = 0,
      },
      {
          .input = "-1",
          .output = INT64_C(-1),
          .code = 0,
      },
      {
          .input = "9223372036854775807",
          .output = INT64_MAX,
          .code = 0,
      },
      {
          .input = "9223372036854775808",
          .code = err_fail,
      },
      {
          .input = "-9223372036854775808",
          .output = INT64_MIN,
          .code = 0,
      },
      {
          .input = "-9223372036854775809",
          .code = err_fail,
      },
      {
          .input = "0x0",
          .code = err_fail,
      },
      {
          .input = "hello",
          .code = err_fail,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->input);
    int64_t r = 0;
    if (TEST_EISG_F(satoi(&str_unmanaged_const(td->input), &r), td->code) && td->code == 0) {
      TEST_CHECK(r == td->output);
    }
  }
}

static void test_str_atou(void) {
  static const struct test_data {
    char const *input;
    int code;
    uint64_t output;
  } test_data[] = {
      {
          .input = "0",
          .output = UINT64_C(0),
          .code = 0,
      },
      {
          .input = "18446744073709551615",
          .output = UINT64_MAX,
          .code = 0,
      },
      {
          .input = "18446744073709551616",
          .code = err_fail,
      },
      {
          .input = "-1",
          .code = err_fail,
      },
      {
          .input = "0x0",
          .code = err_fail,
      },
      {
          .input = "hello",
          .code = err_fail,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->input);
    uint64_t r = 0;
    if (TEST_EISG_F(satou(&str_unmanaged_const(td->input), &r), td->code) && td->code == 0) {
      TEST_CHECK(r == td->output);
    }
  }
}

static void test_str_itoa(void) {
  static const struct test_data {
    int64_t input;
    char const *output;
  } test_data[] = {
      {
          .input = INT64_C(0),
          .output = "0",
      },
      {
          .input = INT64_C(1),
          .output = "1",
      },
      {
          .input = INT64_MAX,
          .output = "9223372036854775807",
      },
      {
          .input = INT64_C(-1),
          .output = "-1",
      },
      {
          .input = INT64_MIN,
          .output = "-9223372036854775808",
      },
  };
  struct str tmp = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->output);
    if (TEST_SUCCEEDED_F(sitoa(td->input, &tmp))) {
      TEST_CHECK(tmp.len > 0);
      TEST_CHECK(strcmp(tmp.ptr, td->output) == 0);
      TEST_MSG("expected %" STR_PH, td->output);
      TEST_MSG("got %" STR_PH, tmp.ptr);
    }
  }
  ereport(sfree(&tmp));
}

static void test_str_utoa(void) {
  static const struct test_data {
    uint64_t input;
    char const *output;
  } test_data[] = {
      {
          .input = UINT64_C(0),
          .output = "0",
      },
      {
          .input = UINT64_MAX,
          .output = "18446744073709551615",
      },
  };

  struct str tmp = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->output);
    if (TEST_SUCCEEDED_F(sutoa(td->input, &tmp))) {
      TEST_CHECK(tmp.len > 0);
      TEST_CHECK(strcmp(tmp.ptr, td->output) == 0);
      TEST_MSG("expected %" STR_PH, td->output);
      TEST_MSG("got %" STR_PH, tmp.ptr);
    }
  }
  ereport(sfree(&tmp));
}

static void test_str_sprintf(void) {
  struct str tmp = {0};
  if (!TEST_SUCCEEDED_F(ssprintf(&tmp, NULL, "hello%04dworld%s", 20, "."))) {
    goto cleanup;
  }
  static char const expected[] = "hello0020world.";
  TEST_CHECK(strcmp(tmp.ptr, expected) == 0);
  TEST_MSG("expected %" STR_PH, expected);
  TEST_MSG("got %" STR_PH, tmp.ptr);
cleanup:
  ereport(sfree(&tmp));
}

#endif // USE_STR

TEST_LIST = {
#ifdef USE_STR
    {"test_str_cpy_free", test_str_cpy_free},
    {"test_str_cpym", test_str_cpym},
    {"test_str_ncpy", test_str_ncpy},
    {"test_str_cat_free", test_str_cat_free},
    {"test_str_catm", test_str_catm},
    {"test_str_ncat", test_str_ncat},
    {"test_str_grow", test_str_grow},
    {"test_str_str", test_str_str},
    {"test_str_replace_all", test_str_replace_all},
    {"test_str_const", test_str_const},
    {"test_str_atoi", test_str_atoi},
    {"test_str_atou", test_str_atou},
    {"test_str_itoa", test_str_itoa},
    {"test_str_utoa", test_str_utoa},
    {"test_str_sprintf", test_str_sprintf},
#endif // USE_STR
    {NULL, NULL},
};
