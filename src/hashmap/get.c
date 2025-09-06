#include "common.h"

void const *ov_hashmap_get(struct ov_hashmap const *const hm, void const *const key_item) {
  if (!hm || !key_item || !hm->map) {
    return NULL;
  }

  return hashmap_get(hm->map, key_item);
}
