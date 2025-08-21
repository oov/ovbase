#include <ovarray.h>
#include <ovtest.h>

#include <inttypes.h>

struct test_item_dynamic {
  wchar_t *key;
  size_t v;
};

static void test_hmap_dynamic_get_key(void const *const item, void const **const key, size_t *const key_bytes) {
  struct test_item_dynamic const *const it = item;
  *key = it->key;
  *key_bytes = it->key ? wcslen(it->key) * sizeof(wchar_t) : 0;
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
  struct test_item_dynamic const *got = NULL;
  if (!TEST_SUCCEEDED_F(hmget(&tmp, &(struct test_item_dynamic){.key = (wchar_t *)L"test1"}, &got))) {
    goto cleanup;
  }
  if (!TEST_CHECK(got == NULL)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmset(&tmp, &((struct test_item_dynamic){.key = (wchar_t *)L"test1", .v = 100}), NULL))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmset(&tmp, &((struct test_item_dynamic){.key = (wchar_t *)L"test2", .v = 200}), NULL))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmcount(&tmp, &count))) {
    goto cleanup;
  }
  if (!TEST_CHECK(count == 2)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmget(&tmp, &(struct test_item_dynamic){.key = (wchar_t *)L"test1"}, &got))) {
    goto cleanup;
  }
  if (!TEST_CHECK(got != NULL)) {
    goto cleanup;
  }
  if (!TEST_CHECK(got->v == 100)) {
    goto cleanup;
  }
  {
    size_t found = 0;
    struct test_item_dynamic *item = NULL;
    for (size_t i = 0; hmiter(&tmp, &i, &item); ++found) {
      TEST_CHECK(item != NULL && (item->v == 100 || item->v == 200));
    }
    TEST_CHECK(found == 2);
  }
  if (!TEST_SUCCEEDED_F(hmdelete(&tmp, &(struct test_item_dynamic){.key = (wchar_t *)L"test1"}, NULL))) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmcount(&tmp, &count))) {
    goto cleanup;
  }
  if (!TEST_CHECK(count == 1)) {
    goto cleanup;
  }
  if (!TEST_SUCCEEDED_F(hmget(&tmp, &(struct test_item_dynamic){.key = (wchar_t *)L"test1"}, &got))) {
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
  struct test_item_static const *got = NULL;
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
  {
    size_t found = 0;
    struct test_item_static *item = NULL;
    for (size_t i = 0; hmiter(&tmp, &i, &item); ++found) {
      TEST_CHECK(item != NULL && (item->v == 100 || item->v == 200));
    }
    TEST_CHECK(found == 2);
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
