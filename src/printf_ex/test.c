#include <ovtest.h>

#include <ovarray.h>
#include <ovprintf_ex.h>

#include <wchar.h>

static void test_ov_sprintf_char(void) {
  char *str = NULL;

  // Basic formatting test
  TEST_CHECK(ov_sprintf_char(&str, NULL, "Hello, %s!", "world"));
  TEST_CHECK(strcmp(str, "Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);
  OV_ARRAY_DESTROY(&str);

  // Number formatting test
  TEST_CHECK(ov_sprintf_char(&str, NULL, "Number: %d", 42));
  TEST_CHECK(strcmp(str, "Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);

  // Reuse buffer test
  TEST_CHECK(ov_sprintf_char(&str, NULL, "First"));
  TEST_CHECK(strcmp(str, "First") == 0);

  TEST_CHECK(ov_sprintf_char(&str, NULL, "Second message is longer"));
  TEST_CHECK(strcmp(str, "Second message is longer") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);

  // NULL format test
  TEST_CHECK(!ov_sprintf_char(&str, NULL, NULL));

  // NULL dest test
  TEST_CHECK(!ov_sprintf_char(NULL, NULL, "test"));
}

static void test_ov_sprintf_wchar(void) {
  wchar_t *str = NULL;

  // Basic formatting test
  TEST_CHECK(ov_sprintf_wchar(&str, NULL, L"Hello, %ls!", L"world"));
  TEST_CHECK(wcscmp(str, L"Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);
  OV_ARRAY_DESTROY(&str);

  // Number formatting test
  TEST_CHECK(ov_sprintf_wchar(&str, NULL, L"Number: %d", 42));
  TEST_CHECK(wcscmp(str, L"Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);

  // Reuse buffer test
  TEST_CHECK(ov_sprintf_wchar(&str, NULL, L"First"));
  TEST_CHECK(wcscmp(str, L"First") == 0);

  TEST_CHECK(ov_sprintf_wchar(&str, NULL, L"Second message is longer"));
  TEST_CHECK(wcscmp(str, L"Second message is longer") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);

  // NULL format test
  TEST_CHECK(!ov_sprintf_wchar(&str, NULL, NULL));

  // NULL dest test
  TEST_CHECK(!ov_sprintf_wchar(NULL, NULL, L"test"));
}

static bool test_vsprintf_char_helper(char **dest, char const *fmt, ...) {
  va_list valist;
  va_start(valist, fmt);
  bool result = ov_vsprintf_char(dest, NULL, fmt, valist);
  va_end(valist);
  return result;
}

static void test_ov_vsprintf_char(void) {
  char *str = NULL;

  TEST_CHECK(test_vsprintf_char_helper(&str, "Value: %d", 123));
  TEST_CHECK(strcmp(str, "Value: 123") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);
}

static bool test_vsprintf_wchar_helper(wchar_t **dest, wchar_t const *fmt, ...) {
  va_list valist;
  va_start(valist, fmt);
  bool result = ov_vsprintf_wchar(dest, NULL, fmt, valist);
  va_end(valist);
  return result;
}

static void test_ov_vsprintf_wchar(void) {
  wchar_t *str = NULL;

  TEST_CHECK(test_vsprintf_wchar_helper(&str, L"Value: %d", 123));
  TEST_CHECK(wcscmp(str, L"Value: 123") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);
}

static void test_ov_sprintf_append_char(void) {
  char *str = NULL;

  // Start with initial string
  TEST_CHECK(ov_sprintf_char(&str, NULL, "Hello"));
  TEST_CHECK(strcmp(str, "Hello") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);

  // Append to existing string
  TEST_CHECK(ov_sprintf_append_char(&str, NULL, ", %s!", "world"));
  TEST_CHECK(strcmp(str, "Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);

  // Append more
  TEST_CHECK(ov_sprintf_append_char(&str, NULL, " Number: %d", 42));
  TEST_CHECK(strcmp(str, "Hello, world! Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  TEST_MSG("Expected length 24, got %zu, string: '%s'", OV_ARRAY_LENGTH(str), str);
  OV_ARRAY_DESTROY(&str);

  // Append to NULL string (should work like normal sprintf)
  str = NULL;
  TEST_CHECK(ov_sprintf_append_char(&str, NULL, "First"));
  TEST_CHECK(strcmp(str, "First") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);
  OV_ARRAY_DESTROY(&str);

  // NULL format test
  TEST_CHECK(!ov_sprintf_append_char(&str, NULL, NULL));

  // NULL dest test
  TEST_CHECK(!ov_sprintf_append_char(NULL, NULL, "test"));
}

static void test_ov_sprintf_append_wchar(void) {
  wchar_t *str = NULL;

  // Start with initial string
  TEST_CHECK(ov_sprintf_wchar(&str, NULL, L"Hello"));
  TEST_CHECK(wcscmp(str, L"Hello") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);

  // Append to existing string
  TEST_CHECK(ov_sprintf_append_wchar(&str, NULL, L", %ls!", L"world"));
  TEST_CHECK(wcscmp(str, L"Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);

  // Append more
  TEST_CHECK(ov_sprintf_append_wchar(&str, NULL, L" Number: %d", 42));
  TEST_CHECK(wcscmp(str, L"Hello, world! Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  TEST_MSG("Expected length 24, got %zu, string: '%ls'", OV_ARRAY_LENGTH(str), str);
  OV_ARRAY_DESTROY(&str);

  // Append to NULL string (should work like normal sprintf)
  str = NULL;
  TEST_CHECK(ov_sprintf_append_wchar(&str, NULL, L"First"));
  TEST_CHECK(wcscmp(str, L"First") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);
  OV_ARRAY_DESTROY(&str);

  // NULL format test
  TEST_CHECK(!ov_sprintf_append_wchar(&str, NULL, NULL));

  // NULL dest test
  TEST_CHECK(!ov_sprintf_append_wchar(NULL, NULL, L"test"));
}

TEST_LIST = {{"ov_sprintf_char", test_ov_sprintf_char},
             {"ov_sprintf_wchar", test_ov_sprintf_wchar},
             {"ov_vsprintf_char", test_ov_vsprintf_char},
             {"ov_vsprintf_wchar", test_ov_vsprintf_wchar},
             {"ov_sprintf_append_char", test_ov_sprintf_append_char},
             {"ov_sprintf_append_wchar", test_ov_sprintf_append_wchar},
             {NULL, NULL}};
