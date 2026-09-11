#include "hmap.h"

bool hmap_iter(struct hmap *const hm, size_t *const i, void **const item) {
  if (!hm || !hm->ptr) {
    return false;
  }
  return hashmap_iter((struct hashmap *)hm->ptr, i, item);
}
