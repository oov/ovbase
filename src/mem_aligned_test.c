#include <ovtest.h>

static bool is_aligned(void *ptr, size_t alignment) { return ((uintptr_t)ptr % alignment) == 0; }

static void test_ov_aligned_alloc_basic(void) {
  void *ptr = NULL;
  struct ov_error err = {0};

  TEST_CHECK(OV_ALIGNED_ALLOC(&ptr, 10, sizeof(int), 16, &err));
  TEST_CHECK(ptr != NULL);
  TEST_CHECK(err.stack[0].info.type == 0 && err.stack[0].info.code == 0 && err.stack[0].info.context == NULL);
  TEST_CHECK(is_aligned(ptr, 16));

  int *int_ptr = (int *)ptr;
  for (int i = 0; i < 10; i++) {
    int_ptr[i] = i * 2;
  }

  for (int i = 0; i < 10; i++) {
    TEST_CHECK(int_ptr[i] == i * 2);
  }

  OV_ALIGNED_FREE(&ptr);
  TEST_CHECK(ptr == NULL);

  OV_ERROR_DESTROY(&err);
}

static void test_ov_aligned_alloc_different_alignments(void) {
  struct ov_error err = {0};

  // Test various alignment sizes
  size_t alignments[] = {4, 8, 16, 32, 64, 128, 256};
  for (size_t i = 0; i < sizeof(alignments) / sizeof(alignments[0]); i++) {
    void *ptr = NULL;
    size_t align = alignments[i];

    TEST_CHECK(OV_ALIGNED_ALLOC(&ptr, 1, sizeof(char), align, &err));
    TEST_CHECK(ptr != NULL);
    TEST_CHECK(err.stack[0].info.type == 0 && err.stack[0].info.code == 0 && err.stack[0].info.context == NULL);
    TEST_CHECK(is_aligned(ptr, align));

    *(char *)ptr = 0x42;
    TEST_CHECK(*(char *)ptr == 0x42);

    OV_ALIGNED_FREE(&ptr);
    TEST_CHECK(ptr == NULL);
  }

  OV_ERROR_DESTROY(&err);
}

static void test_ov_aligned_alloc_error_cases(void) {
  void *ptr = NULL;
  struct ov_error err = {0};

  // Test null pointer
  TEST_CHECK(!OV_ALIGNED_ALLOC(NULL, 10, sizeof(int), 16, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);

  // Test zero size
  err = (struct ov_error){0};
  TEST_CHECK(!OV_ALIGNED_ALLOC(&ptr, 0, sizeof(int), 16, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);

  // Test zero item size
  err = (struct ov_error){0};
  TEST_CHECK(!OV_ALIGNED_ALLOC(&ptr, 10, 0, 16, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);

  // Test invalid alignment (> 256)
  err = (struct ov_error){0};
  TEST_CHECK(!OV_ALIGNED_ALLOC(&ptr, 10, sizeof(int), 512, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);

  // Test non-null pointer
  ptr = (void *)0x12345678; // dummy non-null value
  err = (struct ov_error){0};
  TEST_CHECK(!OV_ALIGNED_ALLOC(&ptr, 10, sizeof(int), 16, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_invalid_argument));
  OV_ERROR_DESTROY(&err);
}

static void test_ov_aligned_free_basic(void) {
  void *ptr = NULL;
  struct ov_error err = {0};

  TEST_CHECK(OV_ALIGNED_ALLOC(&ptr, 5, sizeof(double), 32, &err));
  TEST_CHECK(ptr != NULL);
  TEST_CHECK(err.stack[0].info.type == 0 && err.stack[0].info.code == 0 && err.stack[0].info.context == NULL);
  TEST_CHECK(is_aligned(ptr, 32));

  double *double_ptr = (double *)ptr;
  for (int i = 0; i < 5; i++) {
    double_ptr[i] = i * 3.14;
  }

  OV_ALIGNED_FREE(&ptr);
  TEST_CHECK(ptr == NULL);

  // Test double free (should be safe)
  OV_ALIGNED_FREE(&ptr);
  TEST_CHECK(ptr == NULL);

  // Test free with null pointer
  OV_ALIGNED_FREE(NULL);

  OV_ERROR_DESTROY(&err);
}

TEST_LIST = {{"ov_aligned_alloc_basic", test_ov_aligned_alloc_basic},
             {"ov_aligned_alloc_different_alignments", test_ov_aligned_alloc_different_alignments},
             {"ov_aligned_alloc_error_cases", test_ov_aligned_alloc_error_cases},
             {"ov_aligned_free_basic", test_ov_aligned_free_basic},
             {NULL, NULL}};
