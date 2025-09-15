#include "common.h"
#include <assert.h>

void const *ov_hashmap_get(struct ov_hashmap const *const hm, void const *const key_item) {
  assert(hm != NULL && "hm must not be NULL");
  assert(key_item != NULL && "key_item must not be NULL");
  if (!hm || !key_item) {
    return NULL;
  }

  return hashmap_get(hm->map, key_item);
}
