#include <ovtest.h>

#include <ovarray.h>
#include <ovprintf_ex.h>

#include <string.h>

static void test_ov_sprintf_char2wchar(void) {
  wchar_t *str = NULL;

  // Basic formatting test (ASCII)
  TEST_CHECK(ov_sprintf_char2wchar(&str, NULL, NULL, "Hello, %s!", "world"));
  TEST_CHECK(wcscmp(str, L"Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);
  OV_ARRAY_DESTROY(&str);

  // Number formatting test
  TEST_CHECK(ov_sprintf_char2wchar(&str, NULL, NULL, "Number: %d", 42));
  TEST_CHECK(wcscmp(str, L"Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);

  // UTF-8 input test (Japanese characters)
  TEST_CHECK(ov_sprintf_char2wchar(&str, NULL, NULL, "日本語テスト"));
  TEST_CHECK(wcscmp(str, L"日本語テスト") == 0);
  OV_ARRAY_DESTROY(&str);

  // Mixed ASCII and UTF-8
  TEST_CHECK(ov_sprintf_char2wchar(&str, NULL, NULL, "Hello, %s!", "世界"));
  TEST_CHECK(wcscmp(str, L"Hello, 世界!") == 0);
  OV_ARRAY_DESTROY(&str);

  // Reuse buffer test
  TEST_CHECK(ov_sprintf_char2wchar(&str, NULL, NULL, "First"));
  TEST_CHECK(wcscmp(str, L"First") == 0);

  TEST_CHECK(ov_sprintf_char2wchar(&str, NULL, NULL, "Second message is longer"));
  TEST_CHECK(wcscmp(str, L"Second message is longer") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);
}

static bool vsprintf_char2wchar_helper(wchar_t **dest, char const *fmt, ...) {
  va_list valist;
  va_start(valist, fmt);
  bool result = ov_vsprintf_char2wchar(dest, NULL, NULL, fmt, valist);
  va_end(valist);
  return result;
}

static void test_ov_vsprintf_char2wchar(void) {
  wchar_t *str = NULL;

  TEST_CHECK(vsprintf_char2wchar_helper(&str, "Value: %d", 123));
  TEST_CHECK(wcscmp(str, L"Value: 123") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);
}

static void test_ov_sprintf_append_char2wchar(void) {
  wchar_t *str = NULL;

  // Start with initial string
  TEST_CHECK(ov_sprintf_char2wchar(&str, NULL, NULL, "Hello"));
  TEST_CHECK(wcscmp(str, L"Hello") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);

  // Append to existing string
  TEST_CHECK(ov_sprintf_append_char2wchar(&str, NULL, NULL, ", %s!", "world"));
  TEST_CHECK(wcscmp(str, L"Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);

  // Append more
  TEST_CHECK(ov_sprintf_append_char2wchar(&str, NULL, NULL, " Number: %d", 42));
  TEST_CHECK(wcscmp(str, L"Hello, world! Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);

  // Append to NULL string (should work like normal sprintf)
  str = NULL;
  TEST_CHECK(ov_sprintf_append_char2wchar(&str, NULL, NULL, "First"));
  TEST_CHECK(wcscmp(str, L"First") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);
  OV_ARRAY_DESTROY(&str);

  // UTF-8 append test
  str = NULL;
  TEST_CHECK(ov_sprintf_char2wchar(&str, NULL, NULL, "日本語"));
  TEST_CHECK(ov_sprintf_append_char2wchar(&str, NULL, NULL, "テスト"));
  TEST_CHECK(wcscmp(str, L"日本語テスト") == 0);
  OV_ARRAY_DESTROY(&str);
}

static void test_ov_sprintf_char2wchar_format_mismatch(void) {
  wchar_t *str = NULL;

  // reference and format have different argument types: %d vs %s
  {
    struct ov_error err = {0};
    TEST_CHECK(!ov_sprintf_char2wchar(&str, &err, "%1$d", "%1$s", "hello"));
    TEST_CHECK(str == NULL);
    TEST_CHECK(!ov_error_is(&err, ov_error_type_generic, ov_error_generic_out_of_memory));
    OV_ERROR_DESTROY(&err);
  }

  // append version should also return format mismatch error
  {
    struct ov_error err = {0};
    TEST_CHECK(ov_sprintf_char2wchar(&str, NULL, NULL, "prefix"));
    TEST_CHECK(!ov_sprintf_append_char2wchar(&str, &err, "%1$d", "%1$s", "hello"));
    TEST_CHECK(wcscmp(str, L"prefix") == 0);
    TEST_CHECK(!ov_error_is(&err, ov_error_type_generic, ov_error_generic_out_of_memory));
    OV_ERROR_DESTROY(&err);
    OV_ARRAY_DESTROY(&str);
  }
}

TEST_LIST = {
    {"ov_sprintf_char2wchar", test_ov_sprintf_char2wchar},
    {"ov_vsprintf_char2wchar", test_ov_vsprintf_char2wchar},
    {"ov_sprintf_append_char2wchar", test_ov_sprintf_append_char2wchar},
    {"ov_sprintf_char2wchar_format_mismatch", test_ov_sprintf_char2wchar_format_mismatch},
    {NULL, NULL},
};
