#include "hmap.h"

error hmap_count(struct hmap const *const hm, size_t *const dest) {
  if (!hm) {
    return errg(err_invalid_arugment);
  }
  if (!hm->ptr) {
    *dest = 0;
    return eok();
  }
  *dest = hashmap_count(hm->ptr);
  return eok();
}
