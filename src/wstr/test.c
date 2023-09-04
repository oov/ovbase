#include <ovtest.h>

#include <inttypes.h>

#ifdef USE_WSTR

#  define STR_PH "ls"

static void test_wstr_cpy_free(void) {
  static wchar_t const test_str[] = L"hello";
  struct wstr ws = {0};
  if (TEST_SUCCEEDED_F(scpy(&ws, test_str))) {
    if (TEST_CHECK(ws.ptr != NULL)) {
      TEST_CHECK(wcscmp(ws.ptr, test_str) == 0);
    }
    TEST_CHECK(ws.len == wcslen(test_str));
    TEST_CHECK(ws.cap > wcslen(test_str));
  }

  static wchar_t const test_str2[] = L"good bye world";
  if (TEST_SUCCEEDED_F(scpy(&ws, test_str2))) {
    if (TEST_CHECK(ws.ptr != NULL)) {
      TEST_CHECK(wcscmp(ws.ptr, test_str2) == 0);
    }
    TEST_CHECK(ws.len == wcslen(test_str2));
    TEST_CHECK(ws.cap > wcslen(test_str2));
  }

  static wchar_t const test_str3[] = L"";
  if (TEST_SUCCEEDED_F(scpy(&ws, test_str3))) {
    if (TEST_CHECK(ws.ptr != NULL)) {
      TEST_CHECK(wcscmp(ws.ptr, test_str3) == 0);
    }
    TEST_CHECK(ws.len == wcslen(test_str3));
    TEST_CHECK(ws.cap > wcslen(test_str3));
  }

  if (TEST_SUCCEEDED_F(sfree(&ws))) {
    TEST_CHECK(ws.ptr == NULL);
    TEST_CHECK(ws.len == 0);
    TEST_CHECK(ws.cap == 0);
  }

  TEST_EISG_F(scpy(&ws, NULL), err_invalid_arugment);
}

static void test_wstr_cpym(void) {
  static wchar_t const ws1[] = L"hello";
  static wchar_t const ws2[] = L"world";
  static wchar_t const expected[] = L"helloworld";
  struct wstr ws = {0};
  if (TEST_SUCCEEDED_F(scpym(&ws, ws1, ws2))) {
    TEST_CHECK(wcscmp(ws.ptr, expected) == 0);
    TEST_MSG("expected %" STR_PH, expected);
    TEST_MSG("got %" STR_PH, ws.ptr);
  }
  if (TEST_SUCCEEDED_F(scpym(&ws, ws1, ws2))) {
    TEST_CHECK(wcscmp(ws.ptr, expected) == 0);
    TEST_MSG("expected %" STR_PH, expected);
    TEST_MSG("got %" STR_PH, ws.ptr);
  }
  TEST_SUCCEEDED_F(sfree(&ws));
}

static void test_wstr_ncpy(void) {
  static wchar_t const test_str[] = L"hello";
  struct wstr ws = {0};
  if (TEST_SUCCEEDED_F(sncpy(&ws, test_str, 2))) {
    TEST_CHECK(ws.ptr != NULL);
    TEST_CHECK(ws.len == 2);
    TEST_CHECK(ws.cap > 2);
    TEST_CHECK(ws.ptr[ws.len] == L'\0');
    TEST_SUCCEEDED_F(sfree(&ws));
  }
  if (TEST_SUCCEEDED_F(sncpy(&ws, test_str, 5))) {
    if (TEST_SUCCEEDED_F(sncpy(&ws, test_str, 2))) {
      TEST_CHECK(ws.ptr != NULL);
      TEST_CHECK(ws.len == 2);
      TEST_CHECK(ws.cap > 2);
      TEST_CHECK(ws.ptr[ws.len] == L'\0');
      TEST_SUCCEEDED_F(sfree(&ws));
    }
  }
  if (TEST_SUCCEEDED_F(sncpy(&ws, test_str, 100))) {
    TEST_CHECK(ws.ptr != NULL);
    TEST_CHECK(ws.len == 5);
    TEST_CHECK(ws.cap > 5);
    TEST_CHECK(ws.ptr[ws.len] == L'\0');
    TEST_SUCCEEDED_F(sfree(&ws));
  }
}

static void test_wstr_cat_free(void) {
  static wchar_t const test_str[] = L"hello";
  struct wstr ws = {0};
  TEST_SUCCEEDED_F(scpy(&ws, test_str));
  static wchar_t const test_str2[] = L"world";
  if (TEST_SUCCEEDED_F(scat(&ws, test_str2))) {
    static wchar_t const expected_str[] = L"helloworld";
    if (TEST_CHECK(ws.ptr != NULL)) {
      TEST_CHECK(wcscmp(ws.ptr, expected_str) == 0);
    }
    TEST_CHECK(ws.len == wcslen(expected_str));
    TEST_CHECK(ws.cap > wcslen(expected_str));
  }

  static wchar_t const test_str3[] = L"";
  if (TEST_SUCCEEDED_F(scat(&ws, test_str3))) {
    static wchar_t const expected_str[] = L"helloworld";
    if (TEST_CHECK(ws.ptr != NULL)) {
      TEST_CHECK(wcscmp(ws.ptr, expected_str) == 0);
    }
    TEST_CHECK(ws.len == wcslen(expected_str));
    TEST_CHECK(ws.cap > wcslen(expected_str));
  }

  if (TEST_SUCCEEDED_F(sfree(&ws))) {
    TEST_CHECK(ws.ptr == NULL);
    TEST_CHECK(ws.len == 0);
    TEST_CHECK(ws.cap == 0);
  }

  if (TEST_SUCCEEDED_F(scat(&ws, test_str))) {
    if (TEST_CHECK(ws.ptr != NULL)) {
      TEST_CHECK(wcscmp(ws.ptr, test_str) == 0);
    }
    TEST_CHECK(ws.len == wcslen(test_str));
    TEST_CHECK(ws.cap > wcslen(test_str));
  }
  TEST_SUCCEEDED_F(sfree(&ws));
  TEST_EISG_F(scat(&ws, NULL), err_invalid_arugment);
}

static void test_wstr_catm(void) {
  static wchar_t const ws1[] = L"hello";
  static wchar_t const ws2[] = L"world";
  static wchar_t const expected1[] = L"helloworld";
  static wchar_t const expected2[] = L"helloworldhelloworld";
  struct wstr ws = {0};
  if (TEST_SUCCEEDED_F(scatm(&ws, ws1, ws2))) {
    TEST_CHECK(wcscmp(ws.ptr, expected1) == 0);
    TEST_MSG("expected %" STR_PH, expected1);
    TEST_MSG("got %" STR_PH, ws.ptr);
  }
  if (TEST_SUCCEEDED_F(scatm(&ws, ws1, ws2))) {
    TEST_CHECK(wcscmp(ws.ptr, expected2) == 0);
    TEST_MSG("expected %" STR_PH, expected2);
    TEST_MSG("got %" STR_PH, ws.ptr);
  }
  TEST_SUCCEEDED_F(sfree(&ws));
}

static void test_wstr_ncat(void) {
  static wchar_t const test_str[] = L"hello";
  static wchar_t const test2_str[] = L"world";
  struct wstr ws = {0};
  if (TEST_SUCCEEDED_F(sncat(&ws, test_str, 2))) {
    TEST_CHECK(ws.ptr != NULL);
    TEST_CHECK(ws.len == 2);
    TEST_CHECK(ws.cap > 2);
    TEST_CHECK(wcscmp(ws.ptr, L"he") == 0);
    TEST_SUCCEEDED_F(sfree(&ws));
  }
  if (TEST_SUCCEEDED_F(sncat(&ws, test_str, 100))) {
    TEST_CHECK(ws.ptr != NULL);
    TEST_CHECK(ws.len == 5);
    TEST_CHECK(ws.cap > 5);
    TEST_CHECK(ws.ptr[ws.len] == L'\0');
    TEST_CHECK(wcscmp(ws.ptr, L"hello") == 0);
    TEST_SUCCEEDED_F(sfree(&ws));
  }

  if (TEST_SUCCEEDED_F(sncat(&ws, test_str, 2))) {
    if (TEST_SUCCEEDED_F(sncat(&ws, test2_str, 2))) {
      TEST_CHECK(ws.ptr != NULL);
      TEST_CHECK(ws.len == 4);
      TEST_CHECK(ws.cap > 4);
      TEST_CHECK(wcscmp(ws.ptr, L"hewo") == 0);
    }
    TEST_SUCCEEDED_F(sfree(&ws));
  }
  if (TEST_SUCCEEDED_F(sncat(&ws, test_str, 2))) {
    if (TEST_SUCCEEDED_F(sncat(&ws, test2_str, 100))) {
      TEST_CHECK(ws.ptr != NULL);
      TEST_CHECK(ws.len == 7);
      TEST_CHECK(ws.cap > 7);
      TEST_CHECK(wcscmp(ws.ptr, L"heworld") == 0);
    }
    TEST_SUCCEEDED_F(sfree(&ws));
  }
}

static void test_wstr_grow(void) {
  struct wstr ws = {0};
  if (TEST_SUCCEEDED_F(sgrow(&ws, 1))) {
    TEST_CHECK(ws.ptr != NULL);
    TEST_CHECK(ws.len == 0);
    TEST_CHECK(ws.cap >= 1);
  }
  TEST_SUCCEEDED_F(sfree(&ws));
}

static void test_wstr_str(void) {
  struct wstr ws = wstr_unmanaged(L"hello");
  ptrdiff_t pos = 0, expected = -1;
  if (TEST_SUCCEEDED_F(sstr(&ws, L"!", &pos))) {
    TEST_CHECK(pos == expected);
    TEST_MSG("expected %td", expected);
    TEST_MSG("got %td", pos);
  }
  expected = 1;
  if (TEST_SUCCEEDED_F(sstr(&ws, L"e", &pos))) {
    TEST_CHECK(pos == expected);
    TEST_MSG("expected %td", expected);
    TEST_MSG("got %td", pos);
  }
  expected = 4;
  if (TEST_SUCCEEDED_F(sstr(&ws, L"o", &pos))) {
    TEST_CHECK(pos == expected);
    TEST_MSG("expected %td", expected);
    TEST_MSG("got %td", pos);
  }
  TEST_EISG_F(sstr(&ws, NULL, NULL), err_invalid_arugment);
  TEST_EISG_F(sstr(&ws, L"e", NULL), err_null_pointer);
}

static void test_wstr_replace_all(void) {
  struct wstr ws = {0};
  if (TEST_SUCCEEDED_F(scpy(&ws, L"hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&ws, L"o", L"xx"))) {
      static wchar_t const expected[] = L"hellxx wxxrld";
      TEST_CHECK(wcscmp(ws.ptr, expected) == 0);
      TEST_MSG("expected %" STR_PH, expected);
      TEST_MSG("got %" STR_PH, ws.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&ws, L"hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&ws, L"o", L""))) {
      static wchar_t const expected[] = L"hell wrld";
      TEST_CHECK(wcscmp(ws.ptr, expected) == 0);
      TEST_MSG("expected %" STR_PH, expected);
      TEST_MSG("got %" STR_PH, ws.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&ws, L"hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&ws, L"d", L"xx"))) {
      static wchar_t const expected[] = L"hello worlxx";
      TEST_CHECK(wcscmp(ws.ptr, expected) == 0);
      TEST_MSG("expected %" STR_PH, expected);
      TEST_MSG("got %" STR_PH, ws.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&ws, L"hello world"))) {
    TEST_SUCCEEDED_F(sreplace_all(&ws, L"", L"xx"));
    TEST_SUCCEEDED_F(sreplace_all(&ws, L"a", L"xx"));
    TEST_EISG_F(sreplace_all(&ws, NULL, NULL), err_invalid_arugment);
    TEST_EISG_F(sreplace_all(&ws, L"o", NULL), err_invalid_arugment);
    TEST_EISG_F(sreplace_all(&ws, NULL, L"xx"), err_invalid_arugment);
  }
  TEST_SUCCEEDED_F(sfree(&ws));
}

static void test_wstr_const(void) {
  static wchar_t const ws[] = L"hello";
  ptrdiff_t pos = 0;
  if (TEST_SUCCEEDED_F(sstr(&wstr_unmanaged_const(ws), L"o", &pos))) {
    TEST_CHECK(pos == 4);
  }
}

static void test_wstr_atoi(void) {
  static const struct test_data {
    wchar_t const *input;
    int code;
    int64_t output;
  } test_data[] = {
      {
          .input = L"0",
          .output = INT64_C(0),
          .code = 0,
      },
      {
          .input = L"1",
          .output = INT64_C(1),
          .code = 0,
      },
      {
          .input = L"-1",
          .output = INT64_C(-1),
          .code = 0,
      },
      {
          .input = L"9223372036854775807",
          .output = INT64_MAX,
          .code = 0,
      },
      {
          .input = L"9223372036854775808",
          .code = err_fail,
      },
      {
          .input = L"-9223372036854775808",
          .output = INT64_MIN,
          .code = 0,
      },
      {
          .input = L"-9223372036854775809",
          .code = err_fail,
      },
      {
          .input = L"0x0",
          .code = err_fail,
      },
      {
          .input = L"hello",
          .code = err_fail,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->input);
    int64_t r = 0;
    if (TEST_EISG_F(satoi(&wstr_unmanaged_const(td->input), &r), td->code) && td->code == 0) {
      TEST_CHECK(r == td->output);
    }
  }
}

static void test_wstr_atou(void) {
  static const struct test_data {
    wchar_t const *input;
    int code;
    uint64_t output;
  } test_data[] = {
      {
          .input = L"0",
          .output = UINT64_C(0),
          .code = 0,
      },
      {
          .input = L"18446744073709551615",
          .output = UINT64_MAX,
          .code = 0,
      },
      {
          .input = L"18446744073709551616",
          .code = err_fail,
      },
      {
          .input = L"-1",
          .code = err_fail,
      },
      {
          .input = L"0x0",
          .code = err_fail,
      },
      {
          .input = L"hello",
          .code = err_fail,
      },
  };

  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->input);
    uint64_t r = 0;
    if (TEST_EISG_F(satou(&wstr_unmanaged_const(td->input), &r), td->code) && td->code == 0) {
      TEST_CHECK(r == td->output);
    }
  }
}

static void test_wstr_itoa(void) {
  static const struct test_data {
    int64_t input;
    wchar_t const *output;
  } test_data[] = {
      {
          .input = INT64_C(0),
          .output = L"0",
      },
      {
          .input = INT64_C(1),
          .output = L"1",
      },
      {
          .input = INT64_MAX,
          .output = L"9223372036854775807",
      },
      {
          .input = INT64_C(-1),
          .output = L"-1",
      },
      {
          .input = INT64_MIN,
          .output = L"-9223372036854775808",
      },
  };
  struct wstr tmp = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->output);
    if (TEST_SUCCEEDED_F(sitoa(td->input, &tmp))) {
      TEST_CHECK(tmp.len > 0);
      TEST_CHECK(wcscmp(tmp.ptr, td->output) == 0);
      TEST_MSG("expected %" STR_PH, td->output);
      TEST_MSG("got %" STR_PH, tmp.ptr);
    }
  }
  ereport(sfree(&tmp));
}

static void test_wstr_utoa(void) {
  static const struct test_data {
    uint64_t input;
    wchar_t const *output;
  } test_data[] = {
      {
          .input = UINT64_C(0),
          .output = L"0",
      },
      {
          .input = UINT64_MAX,
          .output = L"18446744073709551615",
      },
  };

  struct wstr tmp = {0};
  size_t n = sizeof(test_data) / sizeof(test_data[0]);
  for (size_t i = 0; i < n; ++i) {
    struct test_data const *const td = test_data + i;
    TEST_CASE_("test #%zu \"%" STR_PH "\"", i, td->output);
    if (TEST_SUCCEEDED_F(sutoa(td->input, &tmp))) {
      TEST_CHECK(tmp.len > 0);
      TEST_CHECK(wcscmp(tmp.ptr, td->output) == 0);
      TEST_MSG("expected %" STR_PH, td->output);
      TEST_MSG("got %" STR_PH, tmp.ptr);
    }
  }
  ereport(sfree(&tmp));
}

static void test_wstr_sprintf(void) {
  struct wstr tmp = {0};
  if (!TEST_SUCCEEDED_F(ssprintf(&tmp, NULL, L"hello%04dworld%s", 20, L"."))) {
    goto cleanup;
  }
  static wchar_t const expected[] = L"hello0020world.";
  TEST_CHECK(wcscmp(tmp.ptr, expected) == 0);
  TEST_MSG("expected %" STR_PH, expected);
  TEST_MSG("got %" STR_PH, tmp.ptr);
cleanup:
  ereport(sfree(&tmp));
}

#endif // USE_WSTR

TEST_LIST = {
#ifdef USE_WSTR
    {"test_wstr_cpy_free", test_wstr_cpy_free},
    {"test_wstr_cpym", test_wstr_cpym},
    {"test_wstr_ncpy", test_wstr_ncpy},
    {"test_wstr_cat_free", test_wstr_cat_free},
    {"test_wstr_catm", test_wstr_catm},
    {"test_wstr_ncat", test_wstr_ncat},
    {"test_wstr_grow", test_wstr_grow},
    {"test_wstr_str", test_wstr_str},
    {"test_wstr_replace_all", test_wstr_replace_all},
    {"test_wstr_const", test_wstr_const},
    {"test_wstr_atoi", test_wstr_atoi},
    {"test_wstr_atou", test_wstr_atou},
    {"test_wstr_itoa", test_wstr_itoa},
    {"test_wstr_utoa", test_wstr_utoa},
    {"test_wstr_sprintf", test_wstr_sprintf},
#endif // USE_WSTR
    {NULL, NULL},
};
