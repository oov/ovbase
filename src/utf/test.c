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

static void test_kick_cesu8(void) {
  static char const u8[] = "\xed\xa0\xbc\xed\xbc\x8d";
  TEST_CHECK(!utf8_to_wchar_len(u8, strlen(u8), NULL));
}

static void test_wchar_to_utf8(void) {
  wchar_t const *ws = L"ABCabc123こんにちは🌏🌍🌎";
  char *golden = "ABCabc123こんにちは🌏🌍🌎";
  char buf[128];
  size_t sz, sz2;
  TEST_CHECK(wchar_to_utf8_len(ws, wcslen(ws), &sz2));
  TEST_CHECK(sz2 == strlen(golden));
  TEST_CHECK(wchar_to_utf8(ws, wcslen(ws), buf, 128, &sz, &sz2));
  TEST_CHECK(sz == wcslen(ws));
  TEST_CHECK(sz2 == strlen(golden));
  buf[sz2] = '\0';
  TEST_CHECK(strcmp(buf, golden) == 0);
}

static void test_kick_broken_surrogate_pair(void) {
  static wchar_t const ws[] = {0xd83c, 0};
  TEST_CHECK(!wchar_to_utf8_len(ws, wcslen(ws), NULL));
}

TEST_LIST = {
    {"test_utf8_to_wchar", test_utf8_to_wchar},
    {"test_kick_cesu8", test_kick_cesu8},
    {"test_wchar_to_utf8", test_wchar_to_utf8},
    {"test_kick_broken_surrogate_pair", test_kick_broken_surrogate_pair},
    {NULL, NULL},
};
