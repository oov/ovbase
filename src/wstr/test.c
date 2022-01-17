#include "ovtest.h"

#include <inttypes.h>

#ifdef USE_WSTR

static void test_wstr_cpy_free(void) {
  static wchar_t const *const test_str = L"hello";
  struct wstr ws = {0};
  if (TEST_SUCCEEDED_F(scpy(&ws, test_str))) {
    if (TEST_CHECK(ws.ptr != NULL)) {
      TEST_CHECK(wcscmp(ws.ptr, test_str) == 0);
    }
    TEST_CHECK(ws.len == wcslen(test_str));
    TEST_CHECK(ws.cap > wcslen(test_str));
  }

  static wchar_t const *const test_str2 = L"good bye world";
  if (TEST_SUCCEEDED_F(scpy(&ws, test_str2))) {
    if (TEST_CHECK(ws.ptr != NULL)) {
      TEST_CHECK(wcscmp(ws.ptr, test_str2) == 0);
    }
    TEST_CHECK(ws.len == wcslen(test_str2));
    TEST_CHECK(ws.cap > wcslen(test_str2));
  }

  static wchar_t const *const test_str3 = L"";
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
  static wchar_t const *const ws1 = L"hello";
  static wchar_t const *const ws2 = L"world";
  static wchar_t const *const expected = L"helloworld";
  struct wstr ws = {0};
  if (TEST_SUCCEEDED_F(scpym(&ws, ws1, ws2))) {
    TEST_CHECK(wcscmp(ws.ptr, expected) == 0);
    TEST_MSG("expected %ls", expected);
    TEST_MSG("got %ls", ws.ptr);
  }
  if (TEST_SUCCEEDED_F(scpym(&ws, ws1, ws2))) {
    TEST_CHECK(wcscmp(ws.ptr, expected) == 0);
    TEST_MSG("expected %ls", expected);
    TEST_MSG("got %ls", ws.ptr);
  }
  TEST_SUCCEEDED_F(sfree(&ws));
}

static void test_wstr_ncpy(void) {
  static wchar_t const *const test_str = L"hello";
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
  static wchar_t const *const test_str = L"hello";
  struct wstr ws = {0};
  TEST_SUCCEEDED_F(scpy(&ws, test_str));
  static wchar_t const *const test_str2 = L"world";
  if (TEST_SUCCEEDED_F(scat(&ws, test_str2))) {
    static wchar_t const *const expected_str = L"helloworld";
    if (TEST_CHECK(ws.ptr != NULL)) {
      TEST_CHECK(wcscmp(ws.ptr, expected_str) == 0);
    }
    TEST_CHECK(ws.len == wcslen(expected_str));
    TEST_CHECK(ws.cap > wcslen(expected_str));
  }

  static wchar_t const *const test_str3 = L"";
  if (TEST_SUCCEEDED_F(scat(&ws, test_str3))) {
    static wchar_t const *const expected_str = L"helloworld";
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
  static wchar_t const *const ws1 = L"hello";
  static wchar_t const *const ws2 = L"world";
  static wchar_t const *const expected1 = L"helloworld";
  static wchar_t const *const expected2 = L"helloworldhelloworld";
  struct wstr ws = {0};
  if (TEST_SUCCEEDED_F(scatm(&ws, ws1, ws2))) {
    TEST_CHECK(wcscmp(ws.ptr, expected1) == 0);
    TEST_MSG("expected %ls", expected1);
    TEST_MSG("got %ls", ws.ptr);
  }
  if (TEST_SUCCEEDED_F(scatm(&ws, ws1, ws2))) {
    TEST_CHECK(wcscmp(ws.ptr, expected2) == 0);
    TEST_MSG("expected %ls", expected2);
    TEST_MSG("got %ls", ws.ptr);
  }
  TEST_SUCCEEDED_F(sfree(&ws));
}

static void test_wstr_ncat(void) {
  static wchar_t const *const test_str = L"hello";
  static wchar_t const *const test2_str = L"world";
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
      static wchar_t const *const expected = L"hellxx wxxrld";
      TEST_CHECK(wcscmp(ws.ptr, expected) == 0);
      TEST_MSG("expected %ls", expected);
      TEST_MSG("got %ls", ws.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&ws, L"hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&ws, L"o", L""))) {
      static wchar_t const *const expected = L"hell wrld";
      TEST_CHECK(wcscmp(ws.ptr, expected) == 0);
      TEST_MSG("expected %ls", expected);
      TEST_MSG("got %ls", ws.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&ws, L"hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&ws, L"d", L"xx"))) {
      static wchar_t const *const expected = L"hello worlxx";
      TEST_CHECK(wcscmp(ws.ptr, expected) == 0);
      TEST_MSG("expected %ls", expected);
      TEST_MSG("got %ls", ws.ptr);
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
  static wchar_t const *const ws = L"hello";
  ptrdiff_t pos = 0;
  if (TEST_SUCCEEDED_F(sstr(&wstr_unmanaged_const(ws), L"o", &pos))) {
    TEST_CHECK(pos == 4);
  }
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
#endif // USE_WSTR
    {NULL, NULL},
};
