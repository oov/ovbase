#include <ovarray.h>
#include <ovhashmap.h>
#include <ovtest.h>

#include <inttypes.h>

struct test_item_dynamic {
  char *key;
  size_t v;
};

static void test_ov_hashmap_dynamic_get_key(void const *const item, void const **const key, size_t *const key_bytes) {
  struct test_item_dynamic const *const it = (struct test_item_dynamic const *)item;
  *key = it->key;
  *key_bytes = it->key ? strlen(it->key) : 0;
}

static void test_ov_hashmap_dynamic(void) {
  struct ov_hashmap *hm = NULL;
  struct ov_error err = {0};
  struct test_item_dynamic const *got = NULL;

  hm = OV_HASHMAP_CREATE_DYNAMIC(sizeof(struct test_item_dynamic), 0, test_ov_hashmap_dynamic_get_key, &err);
  if (!TEST_CHECK(hm != NULL)) {
    goto cleanup;
  }

  if (!TEST_CHECK(OV_HASHMAP_COUNT(hm) == 0)) {
    goto cleanup;
  }

  got = (struct test_item_dynamic const *)OV_HASHMAP_GET(hm, &(struct test_item_dynamic){.key = "test1"});
  if (!TEST_CHECK(got == NULL)) {
    goto cleanup;
  }

  if (!OV_HASHMAP_SET(hm, &((struct test_item_dynamic){.key = "test1", .v = 100}))) {
    TEST_CHECK(false);
    goto cleanup;
  }

  // Test retrieving the set value
  got = (struct test_item_dynamic const *)OV_HASHMAP_GET(hm, &(struct test_item_dynamic){.key = "test1"});
  if (!TEST_CHECK(got != NULL)) {
    goto cleanup;
  }
  if (!TEST_CHECK(got->v == 100)) {
    goto cleanup;
  }

  if (!OV_HASHMAP_SET(hm, &((struct test_item_dynamic){.key = (char *)"test2", .v = 200}))) {
    TEST_CHECK(false);
    goto cleanup;
  }

  if (!TEST_CHECK(OV_HASHMAP_COUNT(hm) == 2)) {
    goto cleanup;
  }

  got = (struct test_item_dynamic const *)OV_HASHMAP_GET(hm, &(struct test_item_dynamic){.key = "test1"});
  if (!TEST_CHECK(got != NULL)) {
    goto cleanup;
  }
  if (!TEST_CHECK(got->v == 100)) {
    goto cleanup;
  }

  {
    size_t found = 0;
    struct test_item_dynamic *item = NULL;
    for (size_t i = 0; OV_HASHMAP_ITER(hm, &i, &item); ++found) {
      TEST_CHECK(item != NULL && (item->v == 100 || item->v == 200));
    }
    TEST_CHECK(found == 2);
  }

  if (!TEST_CHECK(OV_HASHMAP_DELETE(hm, &(struct test_item_dynamic){.key = "test1"}) != NULL)) {
    goto cleanup;
  }

  if (!TEST_CHECK(OV_HASHMAP_COUNT(hm) == 1)) {
    goto cleanup;
  }

  got = (struct test_item_dynamic const *)OV_HASHMAP_GET(hm, &(struct test_item_dynamic){.key = "test1"});
  if (!TEST_CHECK(got == NULL)) {
    goto cleanup;
  }

cleanup:
  OV_HASHMAP_DESTROY(&hm);
  OV_ERROR_DESTROY(&err);
}

static void test_ov_hashmap_static(void) {
  struct test_item_static {
    int64_t key;
    int v;
    int reserved;
  };

  struct ov_hashmap *hm = NULL;
  struct ov_error err = {0};
  struct test_item_static const *got = NULL;

  hm = OV_HASHMAP_CREATE_STATIC(sizeof(struct test_item_static), 0, sizeof(int64_t), &err);
  if (!TEST_CHECK(hm != NULL)) {
    goto cleanup;
  }

  if (!TEST_CHECK(OV_HASHMAP_COUNT(hm) == 0)) {
    goto cleanup;
  }

  got = (struct test_item_static const *)OV_HASHMAP_GET(hm, &(struct test_item_static){.key = 123});
  if (!TEST_CHECK(got == NULL)) {
    goto cleanup;
  }

  if (!OV_HASHMAP_SET(hm, &((struct test_item_static){.key = 123, .v = 100}))) {
    TEST_CHECK(false);
    goto cleanup;
  }

  if (!OV_HASHMAP_SET(hm, &((struct test_item_static){.key = 456, .v = 200}))) {
    TEST_CHECK(false);
    goto cleanup;
  }

  if (!TEST_CHECK(OV_HASHMAP_COUNT(hm) == 2)) {
    goto cleanup;
  }

  got = (struct test_item_static const *)OV_HASHMAP_GET(hm, &(struct test_item_static){.key = 123});
  if (!TEST_CHECK(got != NULL)) {
    goto cleanup;
  }
  if (!TEST_CHECK(got->v == 100)) {
    goto cleanup;
  }

  {
    size_t found = 0;
    struct test_item_static *item = NULL;
    for (size_t i = 0; OV_HASHMAP_ITER(hm, &i, &item); ++found) {
      TEST_CHECK(item != NULL && (item->v == 100 || item->v == 200));
    }
    TEST_CHECK(found == 2);
  }

  if (!TEST_CHECK(OV_HASHMAP_DELETE(hm, &(struct test_item_static){.key = 123}) != NULL)) {
    goto cleanup;
  }

  if (!TEST_CHECK(OV_HASHMAP_COUNT(hm) == 1)) {
    goto cleanup;
  }

  got = (struct test_item_static const *)OV_HASHMAP_GET(hm, &(struct test_item_static){.key = 123});
  if (!TEST_CHECK(got == NULL)) {
    goto cleanup;
  }

cleanup:
  OV_HASHMAP_DESTROY(&hm);
  OV_ERROR_DESTROY(&err);
}

TEST_LIST = {
    {"test_ov_hashmap_dynamic", test_ov_hashmap_dynamic},
    {"test_ov_hashmap_static", test_ov_hashmap_static},
    {NULL, NULL},
};
