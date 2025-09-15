#include <ovtest.h>

#include <ovutf.h>

#ifdef __GNUC__
#  ifndef __has_warning
#    define __has_warning(x) 0
#  endif
#  pragma GCC diagnostic push
#  if __has_warning("-Wc99-compat")
#    pragma GCC diagnostic ignored "-Wc99-compat"
#  endif
#endif // __GNUC__

static size_t char16len(char16_t const *s) {
  size_t len = 0;
  while (*s++) {
    ++len;
  }
  return len;
}

static size_t char32len(char32_t const *s) {
  size_t len = 0;
  while (*s++) {
    ++len;
  }
  return len;
}

static void test_ov_utf8_to_char16(void) {
  char const *u8 = "ABCabc123ﾃｽﾄテストこんにちは𠮷野家🌏🌍🌎";
  char16_t *golden = u"ABCabc123ﾃｽﾄテストこんにちは𠮷野家🌏🌍🌎";
  char16_t buf[128];
  size_t sz;
  TEST_CHECK(ov_utf8_to_char16_len(u8, 9) == 9);
  TEST_CHECK(ov_utf8_to_char16_len(u8, 10) == 0);
  TEST_CHECK(ov_utf8_to_char16_len(u8, strlen(u8)) == char16len(golden));
  TEST_CHECK(ov_utf8_to_char16(u8, strlen(u8), buf, 7, &sz) == 6);
  TEST_CHECK(memcmp(buf, golden, sz * sizeof(char16_t)) == 0);
  TEST_CHECK(ov_utf8_to_char16(u8, strlen(u8), buf, 128, &sz) == char16len(golden));
  TEST_CHECK(sz == strlen(u8));
  TEST_CHECK(char16len(buf) == char16len(golden) && memcmp(buf, golden, char16len(golden) * sizeof(char16_t)) == 0);
}

static void test_ov_utf8_to_char32(void) {
  char const *u8 = "ABCabc123ﾃｽﾄテストこんにちは𠮷野家🌏🌍🌎";
  char32_t *golden = U"ABCabc123ﾃｽﾄテストこんにちは𠮷野家🌏🌍🌎";
  char32_t buf[128];
  size_t sz;
  TEST_CHECK(ov_utf8_to_char32_len(u8, 9) == 9);
  TEST_CHECK(ov_utf8_to_char32_len(u8, 10) == 0);
  TEST_CHECK(ov_utf8_to_char32_len(u8, strlen(u8)) == char32len(golden));
  TEST_CHECK(ov_utf8_to_char32(u8, strlen(u8), buf, 7, &sz) == 6);
  TEST_CHECK(memcmp(buf, golden, sz * sizeof(char32_t)) == 0);
  TEST_CHECK(ov_utf8_to_char32(u8, strlen(u8), buf, 128, &sz) == char32len(golden));
  TEST_CHECK(sz == strlen(u8));
  TEST_CHECK(char32len(buf) == char32len(golden) && memcmp(buf, golden, char32len(golden) * sizeof(char32_t)) == 0);
}

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

static void test_char16_to_utf8(void) {
  char16_t const *ws = u"ABCabc123ﾃｽﾄテストこんにちは🌏🌍🌎";
  char *golden = "ABCabc123ﾃｽﾄテストこんにちは🌏🌍🌎";
  char buf[128];
  size_t sz;
  TEST_CHECK(ov_char16_to_utf8_len(ws, char16len(ws)) == strlen(golden));
  TEST_CHECK(ov_char16_to_utf8(ws, char16len(ws), buf, 128, &sz) == strlen(golden));
  TEST_CHECK(sz == char16len(ws));
  TEST_CHECK(strcmp(buf, golden) == 0);
}

static void test_char16_to_utf8_buffer_safety(void) {
  // Test safe behavior with insufficient buffer (null-terminated string design)
  char16_t const *short_ws = u"ABCD";
  char limited_buf[4] = {'\xff', '\xff', '\xff', '\xff'}; // Only 4 bytes - not enough for 4 chars + null terminator

  // Function should safely write only what fits including null terminator
  size_t sz;
  size_t result = ov_char16_to_utf8(short_ws, char16len(short_ws), limited_buf, 4, &sz);
  TEST_CHECK(result == 3);                     // Should return 3 (wrote "ABC" + null terminator)
  TEST_CHECK(sz == 3);                         // Should read 3 chars from input
  TEST_CHECK(strcmp(limited_buf, "ABC") == 0); // Should be null-terminated "ABC"

  // Verify the function behavior with exact fit (content + null terminator)
  char exact_buf[5];
  result = ov_char16_to_utf8(short_ws, char16len(short_ws), exact_buf, 5, &sz);
  TEST_CHECK(result == 4);                    // Should return 4 (wrote "ABCD" + null terminator)
  TEST_CHECK(sz == 4);                        // Should read all 4 chars from input
  TEST_CHECK(strcmp(exact_buf, "ABCD") == 0); // Should be null-terminated "ABCD"
}

static void test_char32_to_utf8(void) {
  char32_t const *ws = U"ABCabc123ﾃｽﾄテストこんにちは🌏🌍🌎";
  char *golden = "ABCabc123ﾃｽﾄテストこんにちは🌏🌍🌎";
  char buf[128];
  size_t sz;
  TEST_CHECK(ov_char32_to_utf8_len(ws, char32len(ws)) == strlen(golden));
  TEST_CHECK(ov_char32_to_utf8(ws, char32len(ws), buf, 128, &sz) == strlen(golden));
  TEST_CHECK(sz == char32len(ws));
  TEST_CHECK(strcmp(buf, golden) == 0);
}

static void test_char32_to_utf8_buffer_safety(void) {
  // Test safe behavior with insufficient buffer (null-terminated string design)
  char32_t const *short_ws = U"ABCD";
  char limited_buf[4] = {'\xff', '\xff', '\xff', '\xff'}; // Only 4 bytes - not enough for 4 chars + null terminator

  // Function should safely write only what fits including null terminator
  size_t sz;
  size_t result = ov_char32_to_utf8(short_ws, char32len(short_ws), limited_buf, 4, &sz);
  TEST_CHECK(result == 3);                     // Should return 3 (wrote "ABC" + null terminator)
  TEST_CHECK(sz == 3);                         // Should read 3 chars from input
  TEST_CHECK(strcmp(limited_buf, "ABC") == 0); // Should be null-terminated "ABC"

  // Verify the function behavior with exact fit (content + null terminator)
  char exact_buf[5];
  result = ov_char32_to_utf8(short_ws, char32len(short_ws), exact_buf, 5, &sz);
  TEST_CHECK(result == 4);                    // Should return 4 (wrote "ABCD" + null terminator)
  TEST_CHECK(sz == 4);                        // Should read all 4 chars from input
  TEST_CHECK(strcmp(exact_buf, "ABCD") == 0); // Should be null-terminated "ABCD"
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

static void test_wchar_to_utf8_buffer_safety(void) {
  // Test safe behavior with insufficient buffer (null-terminated string design)
  wchar_t const *short_ws = L"ABCD";
  char limited_buf[4] = {'\xff', '\xff', '\xff', '\xff'}; // Only 4 bytes - not enough for 4 chars + null terminator

  // Function should safely write only what fits including null terminator
  size_t sz;
  size_t result = ov_wchar_to_utf8(short_ws, wcslen(short_ws), limited_buf, 4, &sz);
  TEST_CHECK(result == 3);                     // Should return 3 (wrote "ABC" + null terminator)
  TEST_CHECK(sz == 3);                         // Should read 3 chars from input
  TEST_CHECK(strcmp(limited_buf, "ABC") == 0); // Should be null-terminated "ABC"

  // Verify the function behavior with exact fit (content + null terminator)
  char exact_buf[5];
  result = ov_wchar_to_utf8(short_ws, wcslen(short_ws), exact_buf, 5, &sz);
  TEST_CHECK(result == 4);                    // Should return 4 (wrote "ABCD" + null terminator)
  TEST_CHECK(sz == 4);                        // Should read all 4 chars from input
  TEST_CHECK(strcmp(exact_buf, "ABCD") == 0); // Should be null-terminated "ABCD"
}

static void test_ov_sjis_to_utf8_buffer_safety(void) {
  // Test safe behavior with insufficient buffer (null-terminated string design)
  char const *short_sjis = "ABCD";                // Simple ASCII for testing
  char limited_buf[4] = {'\xff', '\xff', '\xff', '\xff'}; // Only 4 bytes - not enough for 4 chars + null terminator

  // Function should safely write only what fits including null terminator
  size_t sz;
  size_t result = ov_sjis_to_utf8(short_sjis, strlen(short_sjis), limited_buf, 4, &sz);
  TEST_CHECK(result == 3);                     // Should return 3 (wrote "ABC" + null terminator)
  TEST_CHECK(sz == 3);                         // Should read 3 chars from input
  TEST_CHECK(strcmp(limited_buf, "ABC") == 0); // Should be null-terminated "ABC"

  // Verify the function behavior with exact fit (content + null terminator)
  char exact_buf[5];
  result = ov_sjis_to_utf8(short_sjis, strlen(short_sjis), exact_buf, 5, &sz);
  TEST_CHECK(result == 4);                    // Should return 4 (wrote "ABCD" + null terminator)
  TEST_CHECK(sz == 4);                        // Should read all 4 chars from input
  TEST_CHECK(strcmp(exact_buf, "ABCD") == 0); // Should be null-terminated "ABCD"
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
    {"test_ov_utf8_to_char16", test_ov_utf8_to_char16},
    {"test_ov_utf8_to_char32", test_ov_utf8_to_char32},
    {"test_ov_utf8_to_wchar", test_ov_utf8_to_wchar},
    {"test_bad_encoding", test_bad_encoding},
    {"test_char16_to_utf8", test_char16_to_utf8},
    {"test_char16_to_utf8_buffer_safety", test_char16_to_utf8_buffer_safety},
    {"test_char32_to_utf8", test_char32_to_utf8},
    {"test_char32_to_utf8_buffer_safety", test_char32_to_utf8_buffer_safety},
    {"test_wchar_to_utf8", test_wchar_to_utf8},
    {"test_wchar_to_utf8_buffer_safety", test_wchar_to_utf8_buffer_safety},
    {"test_kick_broken_surrogate_pair", test_kick_broken_surrogate_pair},
    {"test_ov_sjis_to_utf8", test_ov_sjis_to_utf8},
    {"test_ov_sjis_to_utf8_buffer_safety", test_ov_sjis_to_utf8_buffer_safety},
    {NULL, NULL},
};

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif // __GNUC__
