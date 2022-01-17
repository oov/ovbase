#include "hmap.h"

error hmap_delete(struct hmap *const hm, void const *const key_item, void **const old_item MEM_FILEPOS_PARAMS) {
  if (!hm) {
    return errg(err_invalid_arugment);
  }
  if (!hm->ptr) {
    return eok();
  }
  struct hmap_udata ud = {
      .hm = hm,
#ifdef ALLOCATE_LOGGER
      .filepos = filepos,
#endif
  };
  hashmap_set_udata(hm->ptr, &ud);
  void *r = hashmap_delete(hm->ptr, key_item);
  if (r == NULL) {
    return errg(err_not_found);
  }
  if (old_item) {
    *old_item = r;
  }
  return eok();
}
