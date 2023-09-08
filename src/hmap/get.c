#include "hmap.h"

error hmap_get(struct hmap *const hm, void const *const key_item, void **const item) {
  if (!hm) {
    return errg(err_invalid_arugment);
  }
  if (!hm->ptr) {
    *item = NULL;
    return eok();
  }
  struct hmap_udata ud = {
      .hm = hm,
  };
  hashmap_set_udata(hm->ptr, &ud);
  *item = ov_deconster_(hashmap_get(hm->ptr, key_item));
  return eok();
}
