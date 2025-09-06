#include "common.h"

bool ov_hashmap_iter(struct ov_hashmap *const hm, size_t *const i, void **const item) {
  if (!hm || !i || !item || !hm->map) {
    return false;
  }

  return hashmap_iter(hm->map, i, item);
}
