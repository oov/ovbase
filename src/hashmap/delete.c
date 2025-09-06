#include "common.h"

void const *ov_hashmap_delete(struct ov_hashmap *const hm, void const *const key_item) {
  if (!hm || !key_item || !hm->map) {
    return NULL;
  }

  return hashmap_delete(hm->map, key_item);
}
