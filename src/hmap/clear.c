#include "hmap.h"

error hmap_clear(struct hmap *const hm) {
  if (!hm) {
    return errg(err_invalid_arugment);
  }
  if (hm->ptr) {
    hashmap_clear(hm->ptr, true);
  }
  return eok();
}
