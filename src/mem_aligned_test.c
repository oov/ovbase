#include <ovtest.h>

static bool is_aligned(void *ptr, size_t alignment) { return ((uintptr_t)ptr % alignment) == 0; }

static void test_ov_aligned_alloc_basic(void) {
  void *ptr = NULL;

  TEST_CHECK(OV_ALIGNED_ALLOC(&ptr, 10, sizeof(int), 16));
  TEST_CHECK(ptr != NULL);
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
}

static void test_ov_aligned_alloc_different_alignments(void) {
  // Test various alignment sizes
  size_t alignments[] = {4, 8, 16, 32, 64, 128, 256};
  for (size_t i = 0; i < sizeof(alignments) / sizeof(alignments[0]); i++) {
    void *ptr = NULL;
    size_t align = alignments[i];

    TEST_CHECK(OV_ALIGNED_ALLOC(&ptr, 1, sizeof(char), align));
    TEST_CHECK(ptr != NULL);
    TEST_CHECK(is_aligned(ptr, align));

    *(char *)ptr = 0x42;
    TEST_CHECK(*(char *)ptr == 0x42);

    OV_ALIGNED_FREE(&ptr);
    TEST_CHECK(ptr == NULL);
  }
}

static void test_ov_aligned_alloc_error_cases(void) {
  void *ptr = NULL;

  // Test null pointer
  TEST_CHECK(!OV_ALIGNED_ALLOC(NULL, 10, sizeof(int), 16));

  // Test zero size
  TEST_CHECK(!OV_ALIGNED_ALLOC(&ptr, 0, sizeof(int), 16));

  // Test zero item size
  TEST_CHECK(!OV_ALIGNED_ALLOC(&ptr, 10, 0, 16));

  // Test invalid alignment (> 256)
  TEST_CHECK(!OV_ALIGNED_ALLOC(&ptr, 10, sizeof(int), 512));

  // Test non-null pointer
  ptr = (void *)0x12345678; // dummy non-null value
  TEST_CHECK(!OV_ALIGNED_ALLOC(&ptr, 10, sizeof(int), 16));
}

static void test_ov_aligned_free_basic(void) {
  void *ptr = NULL;

  TEST_CHECK(OV_ALIGNED_ALLOC(&ptr, 5, sizeof(double), 32));
  TEST_CHECK(ptr != NULL);
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
}

TEST_LIST = {{"ov_aligned_alloc_basic", test_ov_aligned_alloc_basic},
             {"ov_aligned_alloc_different_alignments", test_ov_aligned_alloc_different_alignments},
             {"ov_aligned_alloc_error_cases", test_ov_aligned_alloc_error_cases},
             {"ov_aligned_free_basic", test_ov_aligned_free_basic},
             {NULL, NULL}};
