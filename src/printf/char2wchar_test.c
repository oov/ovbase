#include "char2wchar.c"

#include <ovtest.h>

#include <string.h>

enum {
  max_wchars = 128,
};

struct test_context {
  wchar_t buf[max_wchars];
  size_t count;
};

static void test_putc(int const c, void *const ctx) {
  struct test_context *const tc = (struct test_context *)ctx;
  if (tc->count < max_wchars) {
    tc->buf[tc->count++] = (wchar_t)c;
  }
}

static void test_vpprintf_basic(void) {
  struct test_context tc = {0};
  int r = ov_pprintf_char2wchar(test_putc, &tc, NULL, "hello");
  TEST_CHECK(r == 5);
  TEST_MSG("want 5, got %d", r);
  TEST_CHECK(tc.count == 5);
  TEST_MSG("want 5 wchars, got %zu", tc.count);
  TEST_CHECK(tc.buf[0] == L'h');
  TEST_CHECK(tc.buf[1] == L'e');
  TEST_CHECK(tc.buf[2] == L'l');
  TEST_CHECK(tc.buf[3] == L'l');
  TEST_CHECK(tc.buf[4] == L'o');
}

static void test_vpprintf_format(void) {
  struct test_context tc = {0};
  int r = ov_pprintf_char2wchar(test_putc, &tc, NULL, "n=%d", 42);
  TEST_CHECK(r == 4);
  TEST_MSG("want 4, got %d", r);
  TEST_CHECK(tc.count == 4);
  TEST_CHECK(tc.buf[0] == L'n');
  TEST_CHECK(tc.buf[1] == L'=');
  TEST_CHECK(tc.buf[2] == L'4');
  TEST_CHECK(tc.buf[3] == L'2');
}

static void test_vpprintf_unicode(void) {
  struct test_context tc = {0};
  // "あ" is U+3042 (3 bytes in UTF-8)
  int r = ov_pprintf_char2wchar(test_putc, &tc, NULL, "あ");
  TEST_CHECK(r == 3); // 3 UTF-8 bytes returned
  TEST_MSG("want 3, got %d", r);
  TEST_CHECK(tc.count == 1); // 1 wchar_t output
  TEST_MSG("want 1 wchar, got %zu", tc.count);
  TEST_CHECK(tc.buf[0] == L'あ');
  TEST_MSG("want U+3042, got U+%04X", (unsigned)tc.buf[0]);
}

static void test_vpprintf_emoji(void) {
  struct test_context tc = {0};
  // "😀" is U+1F600 (4 bytes in UTF-8)
  int r = ov_pprintf_char2wchar(test_putc, &tc, NULL, "😀");
  TEST_CHECK(r == 4); // 4 UTF-8 bytes returned
  TEST_MSG("want 4, got %d", r);

  // On Windows (sizeof(wchar_t)==2), this is a surrogate pair
  // On Linux (sizeof(wchar_t)==4), this is a single wchar_t
  size_t expected_wchars = (sizeof(wchar_t) == 2) ? 2 : 1;
  TEST_CHECK(tc.count == expected_wchars);
  TEST_MSG("want %zu wchar(s), got %zu", expected_wchars, tc.count);

  if (sizeof(wchar_t) == 2) {
    // Verify surrogate pair
    TEST_CHECK(tc.buf[0] == 0xD83D);
    TEST_CHECK(tc.buf[1] == 0xDE00);
  } else {
    TEST_CHECK(tc.buf[0] == 0x1F600);
  }
}

static void test_vpprintf_mixed(void) {
  struct test_context tc = {0};
  // "Aあ" = 'A' (1 byte) + 'あ' (3 bytes)
  int r = ov_pprintf_char2wchar(test_putc, &tc, NULL, "Aあ");
  TEST_CHECK(r == 4); // 4 UTF-8 bytes
  TEST_MSG("want 4, got %d", r);
  TEST_CHECK(tc.count == 2); // 2 wchar_t
  TEST_MSG("want 2 wchars, got %zu", tc.count);
  TEST_CHECK(tc.buf[0] == L'A');
  TEST_CHECK(tc.buf[1] == L'あ');
}

static void test_snprintf_basic(void) {
  wchar_t buf[32];
  int r = ov_snprintf_char2wchar(buf, 32, NULL, "hello");
  TEST_CHECK(r == 5);
  TEST_MSG("want 5, got %d", r);
  TEST_CHECK(wcscmp(buf, L"hello") == 0);
  TEST_MSG("want 'hello', got '%ls'", buf);
}

static void test_snprintf_unicode(void) {
  wchar_t buf[32];
  int r = ov_snprintf_char2wchar(buf, 32, NULL, "日本語");
  TEST_CHECK(r == 3); // 3 wchar_t output
  TEST_MSG("want 3, got %d", r);
  TEST_CHECK(wcscmp(buf, L"日本語") == 0);
  TEST_MSG("want '日本語', got '%ls'", buf);
}

static void test_snprintf_truncate(void) {
  wchar_t buf[4]; // Only 3 chars + null
  int r = ov_snprintf_char2wchar(buf, 4, NULL, "hello");
  TEST_CHECK(r == 5); // Would need 5 wchars
  TEST_MSG("want 5, got %d", r);
  TEST_CHECK(wcslen(buf) == 3); // Truncated to 3
  TEST_CHECK(wcsncmp(buf, L"hel", 3) == 0);
}

static void test_snprintf_format(void) {
  wchar_t buf[32];
  int r = ov_snprintf_char2wchar(buf, 32, NULL, "[%s]", "テスト");
  TEST_CHECK(r == 5); // '[' + 3 chars + ']' = 5 wchars
  TEST_MSG("want 5, got %d", r);
  TEST_CHECK(wcscmp(buf, L"[テスト]") == 0);
}

static void test_snprintf_null_dest(void) {
  // Should return required size without writing
  int r = ov_snprintf_char2wchar(NULL, 0, NULL, "hello世界");
  // "hello" = 5 wchars, "世界" = 2 wchars = 7 total
  TEST_CHECK(r == 7);
  TEST_MSG("want 7, got %d", r);
}

static void test_order_extension(void) {
  wchar_t buf[32];
  int r = ov_snprintf_char2wchar(buf, 32, NULL, "%2$s%1$d", 42, "X");
  TEST_CHECK(r == 3); // "X42"
  TEST_MSG("want 3, got %d", r);
  TEST_CHECK(wcscmp(buf, L"X42") == 0);
}

static void test_wchar_arg(void) {
  wchar_t buf[32];
  int r = ov_snprintf_char2wchar(buf, 32, NULL, "[%ls]", L"ワイド");
  TEST_CHECK(r == 5); // '[' + 3 + ']'
  TEST_MSG("want 5, got %d", r);
  TEST_CHECK(wcscmp(buf, L"[ワイド]") == 0);
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
    {"test_wchar_arg", test_wchar_arg},
    {NULL, NULL},
};
