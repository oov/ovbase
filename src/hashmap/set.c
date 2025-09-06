#include "common.h"

bool ov_hashmap_set(struct ov_hashmap *const hm, void const *const item MEM_FILEPOS_PARAMS) {
  if (!hm || !item || !hm->map) {
    return false;
  }

#ifdef ALLOCATE_LOGGER
  hm->filepos = filepos;
#endif
  hashmap_set(hm->map, item);
  return !hashmap_oom(hm->map);
}
