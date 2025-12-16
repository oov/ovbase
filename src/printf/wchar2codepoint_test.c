#include "wchar2codepoint.c"

#include <ovtest.h>

#include <inttypes.h>
#include <string.h>

enum {
  max_codepoints = 128,
};

struct test_context {
  int_fast32_t codepoints[max_codepoints];
  size_t count;
};

static void test_putc(int_fast32_t const c, void *const ctx) {
  struct test_context *const tc = (struct test_context *)ctx;
  if (tc->count < max_codepoints) {
    tc->codepoints[tc->count++] = c;
  }
}

static void test_vpprintf_basic(void) {
  struct test_context tc = {0};
  int r = ov_pprintf_wchar2codepoint(test_putc, &tc, NULL, L"hello");
  TEST_CHECK(r == 5);
  TEST_MSG("want 5, got %d", r);
  TEST_CHECK(tc.count == 5);
  TEST_MSG("want 5 codepoints, got %zu", tc.count);
  TEST_CHECK(tc.codepoints[0] == 'h');
  TEST_CHECK(tc.codepoints[1] == 'e');
  TEST_CHECK(tc.codepoints[2] == 'l');
  TEST_CHECK(tc.codepoints[3] == 'l');
  TEST_CHECK(tc.codepoints[4] == 'o');
}

static void test_vpprintf_format(void) {
  struct test_context tc = {0};
  int r = ov_pprintf_wchar2codepoint(test_putc, &tc, NULL, L"n=%d", 42);
  TEST_CHECK(r == 4);
  TEST_MSG("want 4, got %d", r);
  TEST_CHECK(tc.count == 4);
  TEST_CHECK(tc.codepoints[0] == 'n');
  TEST_CHECK(tc.codepoints[1] == '=');
  TEST_CHECK(tc.codepoints[2] == '4');
  TEST_CHECK(tc.codepoints[3] == '2');
}

static void test_vpprintf_unicode(void) {
  struct test_context tc = {0};
  // "あ" is U+3042
  int r = ov_pprintf_wchar2codepoint(test_putc, &tc, NULL, L"あ");
  TEST_CHECK(r == 1); // 1 wchar_t
  TEST_MSG("want 1, got %d", r);
  TEST_CHECK(tc.count == 1); // 1 codepoint
  TEST_MSG("want 1 codepoint, got %zu", tc.count);
  TEST_CHECK(tc.codepoints[0] == 0x3042);
  TEST_MSG("want 0x3042, got 0x%" PRIXFAST32, (uint_fast32_t)tc.codepoints[0]);
}

static void test_vpprintf_emoji(void) {
  struct test_context tc = {0};
  // "😀" is U+1F600
  // On Windows (sizeof(wchar_t)==2), this is a surrogate pair: D83D DE00
  // On Linux (sizeof(wchar_t)==4), this is a single wchar_t
  int r = ov_pprintf_wchar2codepoint(test_putc, &tc, NULL, L"😀");
  int expected_wchars = (sizeof(wchar_t) == 2) ? 2 : 1;
  TEST_CHECK(r == expected_wchars);
  TEST_MSG("want %d, got %d", expected_wchars, r);
  TEST_CHECK(tc.count == 1); // Always 1 codepoint
  TEST_MSG("want 1 codepoint, got %zu", tc.count);
  TEST_CHECK(tc.codepoints[0] == 0x1F600);
  TEST_MSG("want 0x1F600, got 0x%" PRIXFAST32, (uint_fast32_t)tc.codepoints[0]);
}

static void test_vpprintf_mixed(void) {
  struct test_context tc = {0};
  // Mix of ASCII and Unicode
  // "Aあ" = 'A' + 'あ' (U+3042)
  int r = ov_pprintf_wchar2codepoint(test_putc, &tc, NULL, L"Aあ");
  TEST_CHECK(r == 2); // 2 wchar_t
  TEST_MSG("want 2, got %d", r);
  TEST_CHECK(tc.count == 2); // 2 codepoints
  TEST_MSG("want 2 codepoints, got %zu", tc.count);
  TEST_CHECK(tc.codepoints[0] == 'A');
  TEST_CHECK(tc.codepoints[1] == 0x3042);
}

static void test_vpprintf_string_arg(void) {
  struct test_context tc = {0};
  // Test with %ls argument containing Unicode
  int r = ov_pprintf_wchar2codepoint(test_putc, &tc, NULL, L"[%ls]", L"日本語");
  // "日本語" = 3 characters
  TEST_CHECK(r == 5); // '[' + 3 chars + ']' = 5 wchar_t
  TEST_MSG("want 5, got %d", r);
  TEST_CHECK(tc.count == 5); // '[' + 3 chars + ']' = 5 codepoints
  TEST_MSG("want 5 codepoints, got %zu", tc.count);
  TEST_CHECK(tc.codepoints[0] == '[');
  TEST_CHECK(tc.codepoints[1] == 0x65E5); // 日
  TEST_CHECK(tc.codepoints[2] == 0x672C); // 本
  TEST_CHECK(tc.codepoints[3] == 0x8A9E); // 語
  TEST_CHECK(tc.codepoints[4] == ']');
}

static void test_vpprintf_char_arg(void) {
  struct test_context tc = {0};
  // Test with %hs argument (char string)
  int r = ov_pprintf_wchar2codepoint(test_putc, &tc, NULL, L"[%hs]", "ABC");
  TEST_CHECK(r == 5); // '[' + 'A' + 'B' + 'C' + ']' = 5 wchar_t
  TEST_MSG("want 5, got %d", r);
  TEST_CHECK(tc.count == 5); // 5 codepoints
  TEST_MSG("want 5 codepoints, got %zu", tc.count);
  TEST_CHECK(tc.codepoints[0] == '[');
  TEST_CHECK(tc.codepoints[1] == 'A');
  TEST_CHECK(tc.codepoints[2] == 'B');
  TEST_CHECK(tc.codepoints[3] == 'C');
  TEST_CHECK(tc.codepoints[4] == ']');
}

static void test_pprintf_basic(void) {
  struct test_context tc = {0};
  int r = ov_pprintf_wchar2codepoint(test_putc, &tc, NULL, L"%d + %d = %d", 1, 2, 3);
  TEST_CHECK(r == 9); // "1 + 2 = 3"
  TEST_MSG("want 9, got %d", r);
  TEST_CHECK(tc.count == 9);
}

static void test_pprintf_order_extension(void) {
  struct test_context tc = {0};
  int r = ov_pprintf_wchar2codepoint(test_putc, &tc, NULL, L"%2$ls%1$d", 42, L"X");
  TEST_CHECK(r == 3); // "X42"
  TEST_MSG("want 3, got %d", r);
  TEST_CHECK(tc.count == 3);
  TEST_CHECK(tc.codepoints[0] == 'X');
  TEST_CHECK(tc.codepoints[1] == '4');
  TEST_CHECK(tc.codepoints[2] == '2');
}

TEST_LIST = {
    {"test_vpprintf_basic", test_vpprintf_basic},
    {"test_vpprintf_format", test_vpprintf_format},
    {"test_vpprintf_unicode", test_vpprintf_unicode},
    {"test_vpprintf_emoji", test_vpprintf_emoji},
    {"test_vpprintf_mixed", test_vpprintf_mixed},
    {"test_vpprintf_string_arg", test_vpprintf_string_arg},
    {"test_vpprintf_char_arg", test_vpprintf_char_arg},
    {"test_pprintf_basic", test_pprintf_basic},
    {"test_pprintf_order_extension", test_pprintf_order_extension},
    {NULL, NULL},
};
