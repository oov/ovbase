#include <ovtest.h>

#include <ovarray.h>
#include <ovbase.h>

static void test_array(void) {
  int *a = NULL;
  OV_ARRAY_SET_LENGTH(a, 4);
  TEST_CHECK(OV_ARRAY_LENGTH(a) == 0);
  TEST_CHECK(OV_ARRAY_CAPACITY(a) == 0);
  TEST_CHECK(OV_ARRAY_GROW(&a, 4));
  TEST_CHECK(a != NULL);
  TEST_CHECK(OV_ARRAY_LENGTH(a) == 0);
  TEST_CHECK(OV_ARRAY_CAPACITY(a) == 4);
  OV_ARRAY_SET_LENGTH(a, 4);
  TEST_CHECK(OV_ARRAY_LENGTH(a) == 4);
  OV_ARRAY_DESTROY(&a);
  TEST_CHECK(a == NULL);
}

static void test_array_stack(void) {
  int *a = NULL;
  TEST_CHECK(OV_ARRAY_PUSH(&a, 123));
  TEST_CHECK(OV_ARRAY_LENGTH(a) == 1);
  TEST_CHECK(OV_ARRAY_CAPACITY(a) >= 1);

  TEST_CHECK(OV_ARRAY_PUSH(&a, 456));
  TEST_CHECK(OV_ARRAY_LENGTH(a) == 2);
  TEST_CHECK(OV_ARRAY_CAPACITY(a) >= 2);

  TEST_CHECK(OV_ARRAY_POP(a) == 456);
  TEST_CHECK(OV_ARRAY_LENGTH(a) == 1);
  TEST_CHECK(OV_ARRAY_CAPACITY(a) >= 2);

  TEST_CHECK(OV_ARRAY_POP(a) == 123);
  TEST_CHECK(OV_ARRAY_LENGTH(a) == 0);
  TEST_CHECK(OV_ARRAY_CAPACITY(a) >= 2);

  OV_ARRAY_DESTROY(&a);
  TEST_CHECK(a == NULL);
}

static void test_bitarray_growable(void) {
  ov_bitarray *a = NULL;
  OV_BITARRAY_SET_LENGTH(a, 4);
  TEST_CHECK(OV_BITARRAY_LENGTH(a) == 0);
  TEST_CHECK(OV_BITARRAY_CAPACITY(a) == 0);
  TEST_CHECK(OV_BITARRAY_GROW(&a, 4));
  TEST_CHECK(a != NULL);
  TEST_CHECK(OV_BITARRAY_LENGTH(a) == 0);
  TEST_CHECK(OV_BITARRAY_CAPACITY(a) == sizeof(ov_bitarray) * 8);
  TEST_CHECK(OV_ARRAY_CAPACITY(a) == sizeof(ov_bitarray));
  OV_BITARRAY_SET_LENGTH(a, 4);
  TEST_CHECK(OV_BITARRAY_LENGTH(a) == 4);
  OV_BITARRAY_DESTROY(&a);
  TEST_CHECK(a == NULL);
}

static void test_bitarray_fixed_length(void) {
  ov_bitarray *a = NULL;
  TEST_CHECK(OV_BITARRAY_ALLOC(&a, sizeof(ov_bitarray) * 8 * 2));
  TEST_CHECK(a != NULL);

  TEST_CHECK(OV_BITARRAY_GET(a, 0) == false);
  TEST_CHECK(a[0] == 0);
  OV_BITARRAY_SET(a, 0);
  TEST_CHECK(OV_BITARRAY_GET(a, 0) == true);
  TEST_CHECK(a[0] != 0);
  OV_BITARRAY_CLEAR(a, 0);
  TEST_CHECK(OV_BITARRAY_GET(a, 0) == false);
  TEST_CHECK(a[0] == 0);

  {
    size_t const index = sizeof(ov_bitarray) * 8 / 2;
    TEST_CHECK(OV_BITARRAY_GET(a, index) == false);
    TEST_CHECK(a[0] == 0);
    OV_BITARRAY_SET(a, index);
    TEST_CHECK(OV_BITARRAY_GET(a, index) == true);
    TEST_CHECK(OV_BITARRAY_GET(a, 0) == false);
    TEST_CHECK(a[0] != 0);
    OV_BITARRAY_CLEAR(a, index);
    TEST_CHECK(OV_BITARRAY_GET(a, index) == false);
    TEST_CHECK(OV_BITARRAY_GET(a, 0) == false);
    TEST_CHECK(a[0] == 0);
  }

  {
    size_t const index = sizeof(ov_bitarray) * 8;
    TEST_CHECK(OV_BITARRAY_GET(a, index) == false);
    TEST_CHECK(a[1] == 0);
    OV_BITARRAY_SET(a, index);
    TEST_CHECK(OV_BITARRAY_GET(a, index) == true);
    TEST_CHECK(a[1] != 0);
    OV_BITARRAY_CLEAR(a, index);
    TEST_CHECK(OV_BITARRAY_GET(a, index) == false);
    TEST_CHECK(a[1] == 0);
  }

  OV_BITARRAY_FREE(&a);
  TEST_CHECK(a == NULL);
}

static void test_ov_array_grow_success(void) {
  char *arr = NULL;

  TEST_CHECK(OV_ARRAY_GROW(&arr, 10));
  TEST_CHECK(arr != NULL);
  TEST_CHECK(OV_ARRAY_CAPACITY(arr) >= 10);

  OV_ARRAY_DESTROY(&arr);
}

static void test_ov_bitarray_grow_basic(void) {
  ov_bitarray *ba = NULL;

  // Test growing from empty
  TEST_CHECK(OV_BITARRAY_GROW(&ba, 64));
  TEST_CHECK(ba != NULL);
  TEST_CHECK(OV_BITARRAY_CAPACITY(ba) >= 64);

  // Test growing to larger size
  TEST_CHECK(OV_BITARRAY_GROW(&ba, 128));
  TEST_CHECK(OV_BITARRAY_CAPACITY(ba) >= 128);

  // Test growing to same or smaller size (should succeed without reallocation)
  TEST_CHECK(OV_BITARRAY_GROW(&ba, 100));

  OV_ARRAY_DESTROY(&ba);
}

static void test_ov_bitarray_grow_preserves_bits(void) {
  ov_bitarray *ba = NULL;

  // Create initial array and set some bits
  TEST_CHECK(OV_BITARRAY_GROW(&ba, 64));

  OV_BITARRAY_SET_LENGTH(ba, 64);
  OV_BITARRAY_SET(ba, 0);
  OV_BITARRAY_SET(ba, 31);
  OV_BITARRAY_SET(ba, 63);

  TEST_CHECK(OV_BITARRAY_GET(ba, 0));
  TEST_CHECK(OV_BITARRAY_GET(ba, 31));
  TEST_CHECK(OV_BITARRAY_GET(ba, 63));
  TEST_CHECK(!OV_BITARRAY_GET(ba, 1));

  // Grow and verify bits are preserved
  TEST_CHECK(OV_BITARRAY_GROW(&ba, 128));
  TEST_CHECK(OV_BITARRAY_GET(ba, 0));
  TEST_CHECK(OV_BITARRAY_GET(ba, 31));
  TEST_CHECK(OV_BITARRAY_GET(ba, 63));
  TEST_CHECK(!OV_BITARRAY_GET(ba, 64));

  OV_ARRAY_DESTROY(&ba);
}

TEST_LIST = {
    {"test_array", test_array},
    {"test_array_stack", test_array_stack},
    {"test_bitarray_growable", test_bitarray_growable},
    {"test_bitarray_fixed_length", test_bitarray_fixed_length},
    {"test_ov_array_grow_success", test_ov_array_grow_success},
    {"test_ov_bitarray_grow_basic", test_ov_bitarray_grow_basic},
    {"test_ov_bitarray_grow_preserves_bits", test_ov_bitarray_grow_preserves_bits},
    {NULL, NULL},
};
