#include "common.h"

void ov_hashmap_destroy(struct ov_hashmap **const hmp MEM_FILEPOS_PARAMS) {
  if (!hmp || !*hmp) {
    return;
  }

  struct ov_hashmap *hm = *hmp;
  if (hm->map) {
    hashmap_free(hm->map);
    hm->map = NULL;
  }
  ov_mem_free((void **)hmp MEM_FILEPOS_VALUES_PASSTHRU);
}
