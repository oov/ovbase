#include <ovprintf_ex.h>

#include <ovarray.h>
#include <ovtest.h>

#include <wchar.h>

static void test_ov_sprintf_char(void) {
  char *str = NULL;
  struct ov_error err = {0};

  // Basic formatting test
  TEST_CHECK(ov_sprintf_char(&str, NULL, "Hello, %s!", &err, "world"));
  TEST_CHECK(strcmp(str, "Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);
  OV_ARRAY_DESTROY(&str);

  // Number formatting test
  TEST_CHECK(ov_sprintf_char(&str, NULL, "Number: %d", &err, 42));
  TEST_CHECK(strcmp(str, "Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);

  // Reuse buffer test
  TEST_CHECK(ov_sprintf_char(&str, NULL, "First", &err));
  TEST_CHECK(strcmp(str, "First") == 0);

  TEST_CHECK(ov_sprintf_char(&str, NULL, "Second message is longer", &err));
  TEST_CHECK(strcmp(str, "Second message is longer") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);

  // NULL format test
  TEST_CHECK(!ov_sprintf_char(&str, NULL, NULL, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);

  // NULL dest test
  TEST_CHECK(!ov_sprintf_char(NULL, NULL, "test", &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);
}

static void test_ov_sprintf_wchar(void) {
  wchar_t *str = NULL;
  struct ov_error err = {0};

  // Basic formatting test
  TEST_CHECK(ov_sprintf_wchar(&str, NULL, L"Hello, %ls!", &err, L"world"));
  TEST_CHECK(wcscmp(str, L"Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);
  OV_ARRAY_DESTROY(&str);

  // Number formatting test
  TEST_CHECK(ov_sprintf_wchar(&str, NULL, L"Number: %d", &err, 42));
  TEST_CHECK(wcscmp(str, L"Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);

  // Reuse buffer test
  TEST_CHECK(ov_sprintf_wchar(&str, NULL, L"First", &err));
  TEST_CHECK(wcscmp(str, L"First") == 0);

  TEST_CHECK(ov_sprintf_wchar(&str, NULL, L"Second message is longer", &err));
  TEST_CHECK(wcscmp(str, L"Second message is longer") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);

  // NULL format test
  TEST_CHECK(!ov_sprintf_wchar(&str, NULL, NULL, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);

  // NULL dest test
  TEST_CHECK(!ov_sprintf_wchar(NULL, NULL, L"test", &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);
}

static bool test_vsprintf_char_helper(char **dest, struct ov_error *err, char const *fmt, ...) {
  va_list valist;
  va_start(valist, fmt);
  bool result = ov_vsprintf_char(dest, NULL, fmt, err, valist);
  va_end(valist);
  return result;
}

static void test_ov_vsprintf_char(void) {
  char *str = NULL;
  struct ov_error err = {0};

  TEST_CHECK(test_vsprintf_char_helper(&str, &err, "Value: %d", 123));
  TEST_CHECK(strcmp(str, "Value: 123") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);
}

static bool test_vsprintf_wchar_helper(wchar_t **dest, struct ov_error *err, wchar_t const *fmt, ...) {
  va_list valist;
  va_start(valist, fmt);
  bool result = ov_vsprintf_wchar(dest, NULL, fmt, err, valist);
  va_end(valist);
  return result;
}

static void test_ov_vsprintf_wchar(void) {
  wchar_t *str = NULL;
  struct ov_error err = {0};

  TEST_CHECK(test_vsprintf_wchar_helper(&str, &err, L"Value: %d", 123));
  TEST_CHECK(wcscmp(str, L"Value: 123") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);
}

static void test_ov_sprintf_append_char(void) {
  char *str = NULL;
  struct ov_error err = {0};

  // Start with initial string
  TEST_CHECK(ov_sprintf_char(&str, NULL, "Hello", &err));
  TEST_CHECK(strcmp(str, "Hello") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);

  // Append to existing string
  TEST_CHECK(ov_sprintf_append_char(&str, NULL, ", %s!", &err, "world"));
  TEST_CHECK(strcmp(str, "Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);

  // Append more
  TEST_CHECK(ov_sprintf_append_char(&str, NULL, " Number: %d", &err, 42));
  TEST_CHECK(strcmp(str, "Hello, world! Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  TEST_MSG("Expected length 24, got %zu, string: '%s'", OV_ARRAY_LENGTH(str), str);
  OV_ARRAY_DESTROY(&str);

  // Append to NULL string (should work like normal sprintf)
  str = NULL;
  TEST_CHECK(ov_sprintf_append_char(&str, NULL, "First", &err));
  TEST_CHECK(strcmp(str, "First") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);
  OV_ARRAY_DESTROY(&str);

  // NULL format test
  TEST_CHECK(!ov_sprintf_append_char(&str, NULL, NULL, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);

  // NULL dest test
  TEST_CHECK(!ov_sprintf_append_char(NULL, NULL, "test", &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);
}

static void test_ov_sprintf_append_wchar(void) {
  wchar_t *str = NULL;
  struct ov_error err = {0};

  // Start with initial string
  TEST_CHECK(ov_sprintf_wchar(&str, NULL, L"Hello", &err));
  TEST_CHECK(wcscmp(str, L"Hello") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);

  // Append to existing string
  TEST_CHECK(ov_sprintf_append_wchar(&str, NULL, L", %ls!", &err, L"world"));
  TEST_CHECK(wcscmp(str, L"Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);

  // Append more
  TEST_CHECK(ov_sprintf_append_wchar(&str, NULL, L" Number: %d", &err, 42));
  TEST_CHECK(wcscmp(str, L"Hello, world! Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  TEST_MSG("Expected length 24, got %zu, string: '%ls'", OV_ARRAY_LENGTH(str), str);
  OV_ARRAY_DESTROY(&str);

  // Append to NULL string (should work like normal sprintf)
  str = NULL;
  TEST_CHECK(ov_sprintf_append_wchar(&str, NULL, L"First", &err));
  TEST_CHECK(wcscmp(str, L"First") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);
  OV_ARRAY_DESTROY(&str);

  // NULL format test
  TEST_CHECK(!ov_sprintf_append_wchar(&str, NULL, NULL, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);

  // NULL dest test
  TEST_CHECK(!ov_sprintf_append_wchar(NULL, NULL, L"test", &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);
}

TEST_LIST = {{"ov_sprintf_char", test_ov_sprintf_char},
             {"ov_sprintf_wchar", test_ov_sprintf_wchar},
             {"ov_vsprintf_char", test_ov_vsprintf_char},
             {"ov_vsprintf_wchar", test_ov_vsprintf_wchar},
             {"ov_sprintf_append_char", test_ov_sprintf_append_char},
             {"ov_sprintf_append_wchar", test_ov_sprintf_append_wchar},
             {NULL, NULL}};
