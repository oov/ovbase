#include "hmap.h"

error hmap_set(struct hmap *const hm, void const *const item, void **const old_item MEM_FILEPOS_PARAMS) {
  if (!hm || !hm->ptr) {
    return errg(err_invalid_arugment);
  }
  struct hmap_udata ud = {
      .hm = hm,
#ifdef ALLOCATE_LOGGER
      .filepos = filepos,
#endif
  };
  hashmap_set_udata((struct hashmap *)hm->ptr, &ud);
  void *r = ov_deconster_(hashmap_set((struct hashmap *)hm->ptr, item));
  if (r == NULL && hashmap_oom((struct hashmap *)hm->ptr)) {
    return errg(err_out_of_memory);
  }
  if (old_item) {
    *old_item = r;
  }
  return eok();
}
