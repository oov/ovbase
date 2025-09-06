#include "common.h"

void ov_hashmap_clear(struct ov_hashmap *const hm) {
  if (!hm || !hm->map) {
    return;
  }

  hashmap_clear(hm->map, false);
}
