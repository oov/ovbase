#include "common.h"
#include <assert.h>

bool ov_hashmap_iter(struct ov_hashmap *const hm, size_t *const i, void **const item) {
  assert(hm != NULL && "hm must not be NULL");
  assert(i != NULL && "i must not be NULL");
  assert(item != NULL && "item must not be NULL");
  if (!hm || !i || !item) {
    return false;
  }

  return hashmap_iter(hm->map, i, item);
}
