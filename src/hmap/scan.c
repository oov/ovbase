#include "hmap.h"

error hmap_scan(struct hmap *const hm, bool (*iter)(void const *const item, void *const udata), void *const udata) {
  if (!hm || !hm->ptr) {
    return errg(err_invalid_arugment);
  }
  if (!hashmap_scan(hm->ptr, iter, udata)) {
    return errg(err_abort);
  }
  return eok();
}
