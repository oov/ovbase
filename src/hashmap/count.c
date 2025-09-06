#include "common.h"

size_t ov_hashmap_count(struct ov_hashmap const *const hm) {
  if (!hm || !hm->map) {
    return 0;
  }

  return hashmap_count(hm->map);
}
