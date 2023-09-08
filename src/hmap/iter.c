#include "hmap.h"

bool hmap_iter(struct hmap *const hm, size_t *const i, void **const item) {
  if (!hm || !hm->ptr) {
    return false;
  }
  return hashmap_iter(hm->ptr, i, item);
}
