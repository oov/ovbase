#include <ovtest.h>

#include <ovutf.h>

static void test_ov_utf8_to_wchar(void) {
  char const *u8 = "ABCabc123ﾃｽﾄテストこんにちは𠮷野家🌏🌍🌎";
  wchar_t *golden = L"ABCabc123ﾃｽﾄテストこんにちは𠮷野家🌏🌍🌎";
  wchar_t buf[128];
  size_t sz;
  TEST_CHECK(ov_utf8_to_wchar_len(u8, 9) == 9);
  TEST_CHECK(ov_utf8_to_wchar_len(u8, 10) == 0);
  TEST_CHECK(ov_utf8_to_wchar_len(u8, strlen(u8)) == wcslen(golden));
  TEST_CHECK(ov_utf8_to_wchar(u8, strlen(u8), buf, 7, &sz) == 6);
  TEST_CHECK(wcsncmp(buf, golden, sz) == 0);
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
  wchar_t const *ws = L"ABCabc123ﾃｽﾄテストこんにちは🌏🌍🌎";
  char *golden = "ABCabc123ﾃｽﾄテストこんにちは🌏🌍🌎";
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

static void test_ov_sjis_to_utf8(void) {
  char const *sjis =
      "ABCabc123\xc3\xbd\xc4\x83\x65\x83\x58\x83\x67\x82\xc9\x82\xd9\x82\xf1\x82\xb2\x93\xfa\x96\x7b\x8c\xea";
  char const *golden = "ABCabc123ﾃｽﾄテストにほんご日本語";
  char buf[128];
  size_t sz;
  TEST_CHECK(ov_sjis_to_utf8_len(sjis, 14) == 21);
  TEST_CHECK(ov_sjis_to_utf8_len(sjis, strlen(sjis)) == strlen(golden));
  TEST_CHECK(ov_sjis_to_utf8(sjis, strlen(sjis), buf, 7, &sz) == 6);
  TEST_CHECK(strncmp(buf, golden, sz) == 0);
  TEST_CHECK(ov_sjis_to_utf8(sjis, strlen(sjis), buf, 128, &sz) == strlen(golden));
  TEST_CHECK(sz == strlen(sjis));
  TEST_CHECK(strcmp(buf, golden) == 0);
}

TEST_LIST = {
    {"test_ov_utf8_to_wchar", test_ov_utf8_to_wchar},
    {"test_bad_encoding", test_bad_encoding},
    {"test_wchar_to_utf8", test_wchar_to_utf8},
    {"test_kick_broken_surrogate_pair", test_kick_broken_surrogate_pair},
    {"test_ov_sjis_to_utf8", test_ov_sjis_to_utf8},
    {NULL, NULL},
};
