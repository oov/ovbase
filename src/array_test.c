#include "ovtest.h"

#include <inttypes.h>

static void test_array(void) {
  struct {
    int64_t *ptr;
    size_t len;
    size_t cap;
  } arr = {0};
  int64_t v = 0;
  TEST_CHECK(alen(&arr) == 0);
  TEST_CHECK(acap(&arr) == 0);
  TEST_EISG_F(apop(&arr, &v), err_not_found);
  TEST_SUCCEEDED_F(apush(&arr, 100));
  TEST_CHECK(alen(&arr) == 1);
  TEST_CHECK(acap(&arr) >= 1);
  TEST_CHECK(arr.ptr[0] == 100);
  TEST_SUCCEEDED_F(apush(&arr, 200));
  TEST_CHECK(alen(&arr) == 2);
  TEST_CHECK(acap(&arr) >= 2);
  TEST_CHECK(arr.ptr[0] == 100);
  TEST_CHECK(arr.ptr[1] == 200);
  TEST_SUCCEEDED_F(apop(&arr, &v));
  TEST_CHECK(alen(&arr) == 1);
  TEST_CHECK(acap(&arr) >= 1);
  TEST_CHECK(arr.ptr[0] == 100);
  TEST_CHECK(v == 200);
  TEST_SUCCEEDED_F(achop(&arr));
  TEST_CHECK(alen(&arr) == 0);
  TEST_CHECK(acap(&arr) >= 1);
  TEST_SUCCEEDED_F(afree(&arr));
  TEST_CHECK(alen(&arr) == 0);
  TEST_CHECK(acap(&arr) == 0);
  TEST_CHECK(arr.ptr == NULL);
}

TEST_LIST = {
    {"test_array", test_array},
    {NULL, NULL},
};
