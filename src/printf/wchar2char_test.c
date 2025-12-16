#include "wchar2char.c"

#include <ovtest.h>

#include <string.h>

enum {
  max_chars = 256,
};

struct test_context {
  char buf[max_chars];
  size_t count;
};

static void test_putc(int const c, void *const ctx) {
  struct test_context *const tc = (struct test_context *)ctx;
  if (tc->count < max_chars) {
    tc->buf[tc->count++] = (char)c;
  }
}

static void test_vpprintf_basic(void) {
  struct test_context tc = {0};
  int r = ov_pprintf_wchar2char(test_putc, &tc, NULL, L"hello");
  TEST_CHECK(r == 5);
  TEST_MSG("want 5, got %d", r);
  TEST_CHECK(tc.count == 5);
  TEST_MSG("want 5 chars, got %zu", tc.count);
  tc.buf[tc.count] = '\0';
  TEST_CHECK(strcmp(tc.buf, "hello") == 0);
}

static void test_vpprintf_format(void) {
  struct test_context tc = {0};
  int r = ov_pprintf_wchar2char(test_putc, &tc, NULL, L"n=%d", 42);
  TEST_CHECK(r == 4);
  TEST_MSG("want 4, got %d", r);
  TEST_CHECK(tc.count == 4);
  tc.buf[tc.count] = '\0';
  TEST_CHECK(strcmp(tc.buf, "n=42") == 0);
}

static void test_vpprintf_unicode(void) {
  struct test_context tc = {0};
  // "あ" is U+3042 -> 3 bytes in UTF-8 (E3 81 82)
  int r = ov_pprintf_wchar2char(test_putc, &tc, NULL, L"あ");
  TEST_CHECK(r == 1); // 1 wchar_t returned
  TEST_MSG("want 1, got %d", r);
  TEST_CHECK(tc.count == 3); // 3 UTF-8 bytes output
  TEST_MSG("want 3 bytes, got %zu", tc.count);
  tc.buf[tc.count] = '\0';
  TEST_CHECK(strcmp(tc.buf, "あ") == 0);
}

static void test_vpprintf_emoji(void) {
  struct test_context tc = {0};
  // "😀" is U+1F600 -> 4 bytes in UTF-8 (F0 9F 98 80)
  int r = ov_pprintf_wchar2char(test_putc, &tc, NULL, L"😀");
  int expected_wchars = (sizeof(wchar_t) == 2) ? 2 : 1;
  TEST_CHECK(r == expected_wchars);
  TEST_MSG("want %d, got %d", expected_wchars, r);
  TEST_CHECK(tc.count == 4); // Always 4 UTF-8 bytes
  TEST_MSG("want 4 bytes, got %zu", tc.count);
  tc.buf[tc.count] = '\0';
  TEST_CHECK(strcmp(tc.buf, "😀") == 0);
}

static void test_vpprintf_mixed(void) {
  struct test_context tc = {0};
  // "Aあ" = 'A' (1 wchar) + 'あ' (1 wchar) = 1 + 3 = 4 UTF-8 bytes
  int r = ov_pprintf_wchar2char(test_putc, &tc, NULL, L"Aあ");
  TEST_CHECK(r == 2); // 2 wchar_t
  TEST_MSG("want 2, got %d", r);
  TEST_CHECK(tc.count == 4); // 1 + 3 = 4 bytes
  TEST_MSG("want 4 bytes, got %zu", tc.count);
  tc.buf[tc.count] = '\0';
  TEST_CHECK(strcmp(tc.buf, "Aあ") == 0);
}

static void test_snprintf_basic(void) {
  char buf[32];
  int r = ov_snprintf_wchar2char(buf, 32, NULL, L"hello");
  TEST_CHECK(r == 5);
  TEST_MSG("want 5, got %d", r);
  TEST_CHECK(strcmp(buf, "hello") == 0);
  TEST_MSG("want 'hello', got '%s'", buf);
}

static void test_snprintf_unicode(void) {
  char buf[32];
  int r = ov_snprintf_wchar2char(buf, 32, NULL, L"日本語");
  // "日本語" = 3 chars, each 3 bytes = 9 bytes
  TEST_CHECK(r == 9);
  TEST_MSG("want 9, got %d", r);
  TEST_CHECK(strcmp(buf, "日本語") == 0);
  TEST_MSG("want '日本語', got '%s'", buf);
}

static void test_snprintf_truncate(void) {
  char buf[6]; // Only 5 chars + null
  int r = ov_snprintf_wchar2char(buf, 6, NULL, L"hello world");
  TEST_CHECK(r == 11); // Would need 11 bytes
  TEST_MSG("want 11, got %d", r);
  TEST_CHECK(strlen(buf) == 5); // Truncated to 5
  TEST_CHECK(strncmp(buf, "hello", 5) == 0);
}

static void test_snprintf_format(void) {
  char buf[64];
  int r = ov_snprintf_wchar2char(buf, 64, NULL, L"[%ls]", L"テスト");
  // '[' + "テスト" (9 bytes) + ']' = 11 bytes
  TEST_CHECK(r == 11);
  TEST_MSG("want 11, got %d", r);
  TEST_CHECK(strcmp(buf, "[テスト]") == 0);
}

static void test_snprintf_null_dest(void) {
  // Should return required size without writing
  int r = ov_snprintf_wchar2char(NULL, 0, NULL, L"hello世界");
  // "hello" = 5 bytes, "世界" = 2*3 = 6 bytes = 11 total
  TEST_CHECK(r == 11);
  TEST_MSG("want 11, got %d", r);
}

static void test_order_extension(void) {
  char buf[32];
  int r = ov_snprintf_wchar2char(buf, 32, NULL, L"%2$ls%1$d", 42, L"X");
  TEST_CHECK(r == 3); // "X42"
  TEST_MSG("want 3, got %d", r);
  TEST_CHECK(strcmp(buf, "X42") == 0);
}

static void test_char_arg(void) {
  char buf[64];
  int r = ov_snprintf_wchar2char(buf, 64, NULL, L"[%hs]", "narrow");
  TEST_CHECK(r == 8); // '[' + 'narrow' (6) + ']' = 8
  TEST_MSG("want 8, got %d", r);
  TEST_CHECK(strcmp(buf, "[narrow]") == 0);
}

TEST_LIST = {
    {"test_vpprintf_basic", test_vpprintf_basic},
    {"test_vpprintf_format", test_vpprintf_format},
    {"test_vpprintf_unicode", test_vpprintf_unicode},
    {"test_vpprintf_emoji", test_vpprintf_emoji},
    {"test_vpprintf_mixed", test_vpprintf_mixed},
    {"test_snprintf_basic", test_snprintf_basic},
    {"test_snprintf_unicode", test_snprintf_unicode},
    {"test_snprintf_truncate", test_snprintf_truncate},
    {"test_snprintf_format", test_snprintf_format},
    {"test_snprintf_null_dest", test_snprintf_null_dest},
    {"test_order_extension", test_order_extension},
    {"test_char_arg", test_char_arg},
    {NULL, NULL},
};
