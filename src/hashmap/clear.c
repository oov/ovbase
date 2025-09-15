#include "common.h"
#include <assert.h>

void ov_hashmap_clear(struct ov_hashmap *const hm) {
  assert(hm != NULL && "hm must not be NULL");
  if (!hm) {
    return;
  }

  hashmap_clear(hm->map, false);
}
