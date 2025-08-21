#include "hmap.h"

error hmap_set(struct hmap *const hm, void const *const item, void const **const old_item MEM_FILEPOS_PARAMS) {
  if (!hm || !hm->ptr) {
    return errg(err_invalid_arugment);
  }
  struct hmap_udata ud = {
      .hm = hm,
#ifdef ALLOCATE_LOGGER
      .filepos = filepos,
#endif
  };
  hashmap_set_udata(hm->ptr, &ud);
  void const *r = hashmap_set(hm->ptr, item);
  if (r == NULL && hashmap_oom(hm->ptr)) {
    return errg(err_out_of_memory);
  }
  if (old_item) {
    *old_item = r;
  }
  return eok();
}
