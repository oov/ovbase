#include <ovtest.h>

#include <inttypes.h>

struct test_item_dynamic {
  struct NATIVE_STR key;
  size_t v;
};

static void test_hmap_dynamic_get_key(void const *const item, void const **const key, size_t *const key_bytes) {
  struct test_item_dynamic const *const it = item;
  *key = it->key.ptr;
  *key_bytes = it->key.len * sizeof(wchar_t);
}

static void test_hmap_dynamic(void) {
  struct hmap tmp = {0};
  if (!TEST_SUCCEEDED_F(hmfree(&tmp))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmnewd(&tmp, sizeof(struct test_item_dynamic), 0, test_hmap_dynamic_get_key))) {
    goto cleanup;
  }
  size_t count = 0;
  if (!TEST_SUCCEEDED_F(hmcount(&tmp, &count))) {
    goto cleanup;
  }
  if (!TEST_CHECK(count == 0)) {
    goto cleanup;
  }
  struct test_item_dynamic *got = NULL;
  if (!TEST_SUCCEEDED_F(hmget(&tmp, &(struct test_item_dynamic){.key = native_unmanaged(NSTR("test1"))}, &got))) {
    goto cleanup;
  }
  if (!TEST_CHECK(got == NULL)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(
          hmset(&tmp, &((struct test_item_dynamic){.key = native_unmanaged(NSTR("test1")), .v = 100}), NULL))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(
          hmset(&tmp, &((struct test_item_dynamic){.key = native_unmanaged(NSTR("test2")), .v = 200}), NULL))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmcount(&tmp, &count))) {
    goto cleanup;
  }
  if (!TEST_CHECK(count == 2)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmget(&tmp, &(struct test_item_dynamic){.key = native_unmanaged(NSTR("test1"))}, &got))) {
    goto cleanup;
  }
  if (!TEST_CHECK(got != NULL)) {
    goto cleanup;
  }
  if (!TEST_CHECK(got->v == 100)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmdelete(&tmp, &(struct test_item_dynamic){.key = native_unmanaged(NSTR("test1"))}, NULL))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmcount(&tmp, &count))) {
    goto cleanup;
  }
  if (!TEST_CHECK(count == 1)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmget(&tmp, &(struct test_item_dynamic){.key = native_unmanaged(NSTR("test1"))}, &got))) {
    goto cleanup;
  }
  if (!TEST_CHECK(got == NULL)) {
    goto cleanup;
  }
cleanup:
  TEST_SUCCEEDED_F(hmfree(&tmp));
}

static void test_hmap_static(void) {
  struct test_item_static {
    int64_t key;
    int v;
    int reserved;
  };
  struct hmap tmp = {0};
  if (!TEST_SUCCEEDED_F(hmfree(&tmp))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmnews(&tmp, sizeof(struct test_item_static), 0, sizeof(int64_t)))) {
    goto cleanup;
  }
  size_t count = 0;
  if (!TEST_SUCCEEDED_F(hmcount(&tmp, &count))) {
    goto cleanup;
  }
  if (!TEST_CHECK(count == 0)) {
    goto cleanup;
  }
  struct test_item_static *got = NULL;
  if (!TEST_SUCCEEDED_F(hmget(&tmp, &(struct test_item_static){.key = 123}, &got))) {
    goto cleanup;
  }
  if (!TEST_CHECK(got == NULL)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmset(&tmp, &((struct test_item_static){.key = 123, .v = 100}), NULL))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmset(&tmp, &((struct test_item_static){.key = 456, .v = 200}), NULL))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmcount(&tmp, &count))) {
    goto cleanup;
  }
  if (!TEST_CHECK(count == 2)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmget(&tmp, &(struct test_item_static){.key = 123}, &got))) {
    goto cleanup;
  }
  if (!TEST_CHECK(got != NULL)) {
    goto cleanup;
  }
  if (!TEST_CHECK(got->v == 100)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmdelete(&tmp, &(struct test_item_static){.key = 123}, NULL))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmcount(&tmp, &count))) {
    goto cleanup;
  }
  if (!TEST_CHECK(count == 1)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmget(&tmp, &(struct test_item_static){.key = 123}, &got))) {
    goto cleanup;
  }
  if (!TEST_CHECK(got == NULL)) {
    goto cleanup;
  }
cleanup:
  TEST_SUCCEEDED_F(hmfree(&tmp));
}

TEST_LIST = {
    {"test_hmap_dynamic", test_hmap_dynamic},
    {"test_hmap_static", test_hmap_static},
    {NULL, NULL},
};
