#include "common.h"
#include <assert.h>

bool ov_hashmap_set(struct ov_hashmap *const hm, void const *const item MEM_FILEPOS_PARAMS) {
  assert(hm != NULL && "hm must not be NULL");
  assert(item != NULL && "item must not be NULL");
#ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
#endif
  if (!hm || !item) {
    return false;
  }

#ifdef ALLOCATE_LOGGER
  hm->filepos = filepos;
#endif
  hashmap_set(hm->map, item);
  return !hashmap_oom(hm->map);
}
