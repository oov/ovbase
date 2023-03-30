#include "ovtest.h"

#include "ovutf.h"

static void test_ov_utf8_to_wchar(void) {
  char const *u8 = "ABCabc123こんにちは𠮷野家🌏🌍🌎";
  wchar_t *golden = L"ABCabc123こんにちは𠮷野家🌏🌍🌎";
  wchar_t buf[128];
  size_t sz;
  TEST_CHECK(ov_utf8_to_wchar_len(u8, 9) == 9);
  TEST_CHECK(ov_utf8_to_wchar_len(u8, 10) == 0);
  TEST_CHECK(ov_utf8_to_wchar_len(u8, strlen(u8)) == wcslen(golden));
  TEST_CHECK(ov_utf8_to_wchar(u8, strlen(u8), buf, 7, &sz) == 6);
  TEST_CHECK(ov_utf8_to_wchar(u8, strlen(u8), buf, 128, &sz) == wcslen(golden));
  TEST_CHECK(sz == strlen(u8));
  TEST_CHECK(wcscmp(buf, golden) == 0);
}

static void test_bad_encoding(void) {
  char const *str = "\xed\xa0\xbc\xed\xbc\x8d";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xc0\xaf";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xc3\x28";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xe3\x81";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\x80\x81\x82";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xe3\x81\x82\xe3\x82";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xc0\xc0";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xe3\x81\x82\x81";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xed\xa0\x80";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xe3\x81\x82\xe3\x82";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xe3\x81\x82\xc0\xaf";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xf0\x9f\x92\xa9\xf0\x9f";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
  str = "\xf0\x9f\x92\xa9\xc0\xaf";
  TEST_CHECK(ov_utf8_to_wchar_len(str, strlen(str)) == 0);
}

static void test_wchar_to_utf8(void) {
  wchar_t const *ws = L"ABCabc123こんにちは🌏🌍🌎";
  char *golden = "ABCabc123こんにちは🌏🌍🌎";
  char buf[128];
  size_t sz;
  TEST_CHECK(ov_wchar_to_utf8_len(ws, wcslen(ws)) == strlen(golden));
  TEST_CHECK(ov_wchar_to_utf8(ws, wcslen(ws), buf, 128, &sz) == strlen(golden));
  TEST_CHECK(sz == wcslen(ws));
  TEST_CHECK(strcmp(buf, golden) == 0);
}

static void test_kick_broken_surrogate_pair(void) {
  static wchar_t const ws[] = {0xd83c, 0};
  TEST_CHECK(!ov_wchar_to_utf8_len(ws, wcslen(ws)));
}

TEST_LIST = {
    {"test_ov_utf8_to_wchar", test_ov_utf8_to_wchar},
    {"test_bad_encoding", test_bad_encoding},
    {"test_wchar_to_utf8", test_wchar_to_utf8},
    {"test_kick_broken_surrogate_pair", test_kick_broken_surrogate_pair},
    {NULL, NULL},
};
