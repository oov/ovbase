#include "ovtest.h"

#include "ovutf.h"

static void test_utf8_to_wchar(void) {
  char const *u8 = "ABCabc123こんにちは🌏🌍🌎";
  wchar_t *golden = L"ABCabc123こんにちは🌏🌍🌎";
  wchar_t buf[128];
  size_t sz, sz2;
  TEST_CHECK(utf8_to_wchar_len(u8, strlen(u8), &sz2));
  TEST_CHECK(sz2 == wcslen(golden));
  TEST_CHECK(utf8_to_wchar(u8, strlen(u8), buf, 128, &sz, &sz2));
  TEST_CHECK(sz == strlen(u8));
  TEST_CHECK(sz2 == wcslen(golden));
  buf[sz2] = L'\0';
  TEST_CHECK(wcscmp(buf, golden) == 0);
}

TEST_LIST = {
    {"test_utf8_to_wchar", test_utf8_to_wchar},
    {NULL, NULL},
};
