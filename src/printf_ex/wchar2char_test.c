#include <ovtest.h>

#include <ovarray.h>
#include <ovprintf_ex.h>

#include <string.h>

static void test_ov_sprintf_wchar2char(void) {
  char *str = NULL;

  // Basic formatting test (ASCII)
  TEST_CHECK(ov_sprintf_wchar2char(&str, NULL, NULL, L"Hello, %ls!", L"world"));
  TEST_CHECK(strcmp(str, "Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);
  OV_ARRAY_DESTROY(&str);

  // Number formatting test
  TEST_CHECK(ov_sprintf_wchar2char(&str, NULL, NULL, L"Number: %d", 42));
  TEST_CHECK(strcmp(str, "Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);

  // Wide character input test (Japanese characters)
  TEST_CHECK(ov_sprintf_wchar2char(&str, NULL, NULL, L"日本語テスト"));
  TEST_CHECK(strcmp(str, "日本語テスト") == 0);
  OV_ARRAY_DESTROY(&str);

  // Mixed ASCII and wide characters
  TEST_CHECK(ov_sprintf_wchar2char(&str, NULL, NULL, L"Hello, %ls!", L"世界"));
  TEST_CHECK(strcmp(str, "Hello, 世界!") == 0);
  OV_ARRAY_DESTROY(&str);

  // Reuse buffer test
  TEST_CHECK(ov_sprintf_wchar2char(&str, NULL, NULL, L"First"));
  TEST_CHECK(strcmp(str, "First") == 0);

  TEST_CHECK(ov_sprintf_wchar2char(&str, NULL, NULL, L"Second message is longer"));
  TEST_CHECK(strcmp(str, "Second message is longer") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);
}

static bool vsprintf_wchar2char_helper(char **dest, wchar_t const *fmt, ...) {
  va_list valist;
  va_start(valist, fmt);
  bool result = ov_vsprintf_wchar2char(dest, NULL, NULL, fmt, valist);
  va_end(valist);
  return result;
}

static void test_ov_vsprintf_wchar2char(void) {
  char *str = NULL;

  TEST_CHECK(vsprintf_wchar2char_helper(&str, L"Value: %d", 123));
  TEST_CHECK(strcmp(str, "Value: 123") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);
}

static void test_ov_sprintf_append_wchar2char(void) {
  char *str = NULL;

  // Start with initial string
  TEST_CHECK(ov_sprintf_wchar2char(&str, NULL, NULL, L"Hello"));
  TEST_CHECK(strcmp(str, "Hello") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);

  // Append to existing string
  TEST_CHECK(ov_sprintf_append_wchar2char(&str, NULL, NULL, L", %ls!", L"world"));
  TEST_CHECK(strcmp(str, "Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);

  // Append more
  TEST_CHECK(ov_sprintf_append_wchar2char(&str, NULL, NULL, L" Number: %d", 42));
  TEST_CHECK(strcmp(str, "Hello, world! Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);

  // Append to NULL string (should work like normal sprintf)
  str = NULL;
  TEST_CHECK(ov_sprintf_append_wchar2char(&str, NULL, NULL, L"First"));
  TEST_CHECK(strcmp(str, "First") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);
  OV_ARRAY_DESTROY(&str);

  // Wide character append test
  str = NULL;
  TEST_CHECK(ov_sprintf_wchar2char(&str, NULL, NULL, L"日本語"));
  TEST_CHECK(ov_sprintf_append_wchar2char(&str, NULL, NULL, L"テスト"));
  TEST_CHECK(strcmp(str, "日本語テスト") == 0);
  OV_ARRAY_DESTROY(&str);
}

static void test_ov_sprintf_wchar2char_format_mismatch(void) {
  char *str = NULL;

  // reference and format have different argument types: %d vs %ls
  {
    struct ov_error err = {0};
    TEST_CHECK(!ov_sprintf_wchar2char(&str, &err, L"%1$d", L"%1$ls", L"hello"));
    TEST_CHECK(str == NULL);
    TEST_CHECK(!ov_error_is(&err, ov_error_type_generic, ov_error_generic_out_of_memory));
    OV_ERROR_DESTROY(&err);
  }

  // append version should also return format mismatch error
  {
    struct ov_error err = {0};
    TEST_CHECK(ov_sprintf_wchar2char(&str, NULL, NULL, L"prefix"));
    TEST_CHECK(!ov_sprintf_append_wchar2char(&str, &err, L"%1$d", L"%1$ls", L"hello"));
    TEST_CHECK(strcmp(str, "prefix") == 0);
    TEST_CHECK(!ov_error_is(&err, ov_error_type_generic, ov_error_generic_out_of_memory));
    OV_ERROR_DESTROY(&err);
    OV_ARRAY_DESTROY(&str);
  }
}

TEST_LIST = {
    {"ov_sprintf_wchar2char", test_ov_sprintf_wchar2char},
    {"ov_vsprintf_wchar2char", test_ov_vsprintf_wchar2char},
    {"ov_sprintf_append_wchar2char", test_ov_sprintf_append_wchar2char},
    {"ov_sprintf_wchar2char_format_mismatch", test_ov_sprintf_wchar2char_format_mismatch},
    {NULL, NULL},
};
