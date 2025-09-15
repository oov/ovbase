#include "common.h"
#include <assert.h>

void ov_hashmap_destroy(struct ov_hashmap **const hmp MEM_FILEPOS_PARAMS) {
  assert(hmp != NULL && "hmp must not be NULL");
#ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
#endif
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
