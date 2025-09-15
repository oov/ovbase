#include "common.h"
#include <assert.h>

void const *ov_hashmap_delete(struct ov_hashmap *const hm, void const *const key_item) {
  assert(hm != NULL && "hm must not be NULL");
  assert(key_item != NULL && "key_item must not be NULL");
  if (!hm || !key_item) {
    return NULL;
  }

  return hashmap_delete(hm->map, key_item);
}
