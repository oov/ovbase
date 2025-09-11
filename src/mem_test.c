#include <ovtest.h>

static void test_ov_realloc_basic(void) {
  void *ptr = NULL;
  struct ov_error err = {0};

  TEST_CHECK(OV_REALLOC(&ptr, 10, sizeof(int), &err));
  TEST_CHECK(ptr != NULL);
  TEST_CHECK(err.stack[0].info.type == 0 && err.stack[0].info.code == 0 && err.stack[0].info.context == NULL);

  int *int_ptr = (int *)ptr;
  for (int i = 0; i < 10; i++) {
    int_ptr[i] = i * 2;
  }

  TEST_CHECK(OV_REALLOC(&ptr, 20, sizeof(int), &err));
  TEST_CHECK(ptr != NULL);
  TEST_CHECK(err.stack[0].info.type == 0 && err.stack[0].info.code == 0 && err.stack[0].info.context == NULL);

  int_ptr = (int *)ptr;
  for (int i = 0; i < 10; i++) {
    TEST_CHECK(int_ptr[i] == i * 2);
  }

  OV_FREE(&ptr);
  TEST_CHECK(ptr == NULL);

  OV_ERROR_DESTROY(&err);
}

static void test_ov_realloc_error_cases(void) {
  void *ptr = NULL;
  struct ov_error err = {0};

  TEST_CHECK(!OV_REALLOC(NULL, 10, sizeof(int), &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);

  err = (struct ov_error){0};
  TEST_CHECK(!OV_REALLOC(&ptr, 10, 0, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);
}

static void test_ov_free_basic(void) {
  void *ptr = NULL;
  struct ov_error err = {0};

  TEST_CHECK(OV_REALLOC(&ptr, 5, sizeof(double), &err));
  TEST_CHECK(ptr != NULL);
  TEST_CHECK(err.stack[0].info.type == 0 && err.stack[0].info.code == 0 && err.stack[0].info.context == NULL);

  double *double_ptr = (double *)ptr;
  for (int i = 0; i < 5; i++) {
    double_ptr[i] = i * 3.14;
  }

  OV_FREE(&ptr);
  TEST_CHECK(ptr == NULL);

  OV_FREE(&ptr);

  OV_ERROR_DESTROY(&err);
}

TEST_LIST = {{"ov_realloc_basic", test_ov_realloc_basic},
             {"ov_realloc_error_cases", test_ov_realloc_error_cases},
             {"ov_free_basic", test_ov_free_basic},
             {NULL, NULL}};
