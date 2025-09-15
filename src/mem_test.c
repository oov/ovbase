#include <ovtest.h>

static void test_ov_realloc_basic(void) {
  void *ptr = NULL;

  TEST_CHECK(OV_REALLOC(&ptr, 10, sizeof(int)));
  TEST_CHECK(ptr != NULL);

  int *int_ptr = (int *)ptr;
  for (int i = 0; i < 10; i++) {
    int_ptr[i] = i * 2;
  }

  TEST_CHECK(OV_REALLOC(&ptr, 20, sizeof(int)));
  TEST_CHECK(ptr != NULL);

  int_ptr = (int *)ptr;
  for (int i = 0; i < 10; i++) {
    TEST_CHECK(int_ptr[i] == i * 2);
  }

  OV_FREE(&ptr);
  TEST_CHECK(ptr == NULL);
}

static void test_ov_free_basic(void) {
  void *ptr = NULL;

  TEST_CHECK(OV_REALLOC(&ptr, 5, sizeof(double)));
  TEST_CHECK(ptr != NULL);

  double *double_ptr = (double *)ptr;
  for (int i = 0; i < 5; i++) {
    double_ptr[i] = i * 3.14;
  }

  OV_FREE(&ptr);
  TEST_CHECK(ptr == NULL);

  OV_FREE(&ptr);
}

TEST_LIST = {
    {"ov_realloc_basic", test_ov_realloc_basic},
    {"ov_free_basic", test_ov_free_basic},
    {NULL, NULL},
};
