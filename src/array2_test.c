#include <ovtest.h>

#include <ovarray.h>

static void test_array(void) {
  int *a = NULL;
  OV_ARRAY_SET_LENGTH(a, 4);
  TEST_CHECK(OV_ARRAY_LENGTH(a) == 0);
  TEST_CHECK(OV_ARRAY_CAPACITY(a) == 0);
  TEST_SUCCEEDED_F(OV_ARRAY_GROW(&a, 4));
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
  TEST_SUCCEEDED_F(OV_ARRAY_PUSH(&a, 123));
  TEST_CHECK(OV_ARRAY_LENGTH(a) == 1);
  TEST_CHECK(OV_ARRAY_CAPACITY(a) >= 1);

  TEST_SUCCEEDED_F(OV_ARRAY_PUSH(&a, 456));
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
  TEST_SUCCEEDED_F(OV_BITARRAY_GROW(&a, 4));
  TEST_CHECK(a != NULL);
  TEST_CHECK(OV_BITARRAY_LENGTH(a) == 0);
  TEST_CHECK(OV_BITARRAY_CAPACITY(a) == sizeof(size_t) * 8);
  TEST_CHECK(OV_ARRAY_CAPACITY(a) == 1);
  OV_BITARRAY_SET_LENGTH(a, 4);
  TEST_CHECK(OV_BITARRAY_LENGTH(a) == 4);
  OV_BITARRAY_DESTROY(&a);
  TEST_CHECK(a == NULL);
}

static void test_bitarray_fixed_length(void) {
  ov_bitarray *a = NULL;
  TEST_SUCCEEDED_F(OV_BITARRAY_ALLOC(&a, sizeof(ov_bitarray) * 8 * 2));
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

TEST_LIST = {
    {"test_array", test_array},
    {"test_array_stack", test_array_stack},
    {"test_bitarray_growable", test_bitarray_growable},
    {"test_bitarray_fixed_length", test_bitarray_fixed_length},
    {NULL, NULL},
};
