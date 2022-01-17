#include "ovtest.h"

#include <inttypes.h>

static size_t err_count(error e) {
  size_t n = 0;
  while (e != NULL) {
    ++n;
    e = e->next;
  }
  return n;
}

static void test_error(void) {
  {
    error e = NULL;
    TEST_CHECK(err_count(e) == 0);
  }
  {
    error e = errg(err_fail);
    TEST_EISG(e, err_fail);
    size_t n = err_count(e);
    TEST_CHECK(n == 1);
    TEST_MSG("expected %d", 1);
    TEST_MSG("got %zu", n);
    efree(&e);
  }
  {
    error e = errg(err_fail);
    e = ethru(e);
    TEST_EISG(e, err_fail);
    TEST_EISG(e->next, err_pass_through);
    size_t n = err_count(e);
    TEST_CHECK(n == 2);
    TEST_MSG("expected %d", 2);
    TEST_MSG("got %zu", n);
    efree(&e);
  }
}

static void test_mem(void) {
  void *p = NULL;
  TEST_SUCCEEDED_F(mem(&p, 8, 1));
  TEST_CHECK(p != NULL);

  TEST_SUCCEEDED_F(mem(&p, 16, 1));
  TEST_CHECK(p != NULL);

  TEST_SUCCEEDED_F(mem(&p, 0, 1));
  TEST_CHECK(p == NULL);

  TEST_SUCCEEDED_F(mem(&p, 8, 1));
  TEST_CHECK(p != NULL);

  TEST_SUCCEEDED_F(mem_free(&p));
  TEST_CHECK(p == NULL);

  TEST_EISG_F(mem(&p, 1, 0), err_invalid_arugment);
  TEST_EISG_F(mem(NULL, 1, 1), err_invalid_arugment);
  TEST_EISG_F(mem_free(NULL), err_invalid_arugment);
}

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
    {"test_error", test_error},
    {"test_mem", test_mem},
    {"test_array", test_array},
    {NULL, NULL},
};
