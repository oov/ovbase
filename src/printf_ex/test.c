#include <ovprintf_ex.h>

#include <ovarray.h>
#include <ovtest.h>

static void test_ov_sprintf_char(void) {
  char *str = NULL;
  error err = eok();

  // Basic formatting test
  err = ov_sprintf_char(&str, NULL, "Hello, %s!", "world");
  TEST_CHECK(esucceeded(err));
  TEST_CHECK(strcmp(str, "Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);
  OV_ARRAY_DESTROY(&str);

  // Number formatting test
  err = ov_sprintf_char(&str, NULL, "Number: %d", 42);
  TEST_CHECK(esucceeded(err));
  TEST_CHECK(strcmp(str, "Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);

  // Reuse buffer test
  err = ov_sprintf_char(&str, NULL, "First");
  TEST_CHECK(esucceeded(err));
  TEST_CHECK(strcmp(str, "First") == 0);

  err = ov_sprintf_char(&str, NULL, "Second message is longer");
  TEST_CHECK(esucceeded(err));
  TEST_CHECK(strcmp(str, "Second message is longer") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);

  // NULL format test
  err = ov_sprintf_char(&str, NULL, NULL);
  TEST_CHECK(efailed(err));
  TEST_CHECK(eisg(err, err_invalid_arugment));
  efree(&err);

  // NULL dest test
  err = ov_sprintf_char(NULL, NULL, "test");
  TEST_CHECK(efailed(err));
  TEST_CHECK(eisg(err, err_invalid_arugment));
  efree(&err);
}

static void test_ov_sprintf_wchar(void) {
  wchar_t *str = NULL;
  error err = eok();

  // Basic formatting test
  err = ov_sprintf_wchar(&str, NULL, L"Hello, %ls!", L"world");
  TEST_CHECK(esucceeded(err));
  TEST_CHECK(wcscmp(str, L"Hello, world!") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);
  OV_ARRAY_DESTROY(&str);

  // Number formatting test
  err = ov_sprintf_wchar(&str, NULL, L"Number: %d", 42);
  TEST_CHECK(esucceeded(err));
  TEST_CHECK(wcscmp(str, L"Number: 42") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);

  // Reuse buffer test
  err = ov_sprintf_wchar(&str, NULL, L"First");
  TEST_CHECK(esucceeded(err));
  TEST_CHECK(wcscmp(str, L"First") == 0);

  err = ov_sprintf_wchar(&str, NULL, L"Second message is longer");
  TEST_CHECK(esucceeded(err));
  TEST_CHECK(wcscmp(str, L"Second message is longer") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);

  // NULL format test
  err = ov_sprintf_wchar(&str, NULL, NULL);
  TEST_CHECK(efailed(err));
  TEST_CHECK(eisg(err, err_invalid_arugment));
  efree(&err);

  // NULL dest test
  err = ov_sprintf_wchar(NULL, NULL, L"test");
  TEST_CHECK(efailed(err));
  TEST_CHECK(eisg(err, err_invalid_arugment));
  efree(&err);
}

static void test_ov_vsprintf_char(void) {
  char *str = NULL;
  error err = eok();

  // Helper function for va_list test
  auto error test_vsprintf_helper(char **dest, char const *fmt, ...) {
    va_list valist;
    va_start(valist, fmt);
    error result = ov_vsprintf_char(dest, NULL, fmt, valist);
    va_end(valist);
    return result;
  };

  err = test_vsprintf_helper(&str, "Value: %d", 123);
  TEST_CHECK(esucceeded(err));
  TEST_CHECK(strcmp(str, "Value: 123") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);
}

static void test_ov_vsprintf_wchar(void) {
  wchar_t *str = NULL;
  error err = eok();

  // Helper function for va_list test
  auto error test_vsprintf_helper(wchar_t * *dest, wchar_t const *fmt, ...) {
    va_list valist;
    va_start(valist, fmt);
    error result = ov_vsprintf_wchar(dest, NULL, fmt, valist);
    va_end(valist);
    return result;
  };

  err = test_vsprintf_helper(&str, L"Value: %d", 123);
  TEST_CHECK(esucceeded(err));
  TEST_CHECK(wcscmp(str, L"Value: 123") == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);
}

TEST_LIST = {{"ov_sprintf_char", test_ov_sprintf_char},
             {"ov_sprintf_wchar", test_ov_sprintf_wchar},
             {"ov_vsprintf_char", test_ov_vsprintf_char},
             {"ov_vsprintf_wchar", test_ov_vsprintf_wchar},
             {NULL, NULL}};
