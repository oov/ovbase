#include "hmap.h"

error hmap_free(struct hmap *const hm MEM_FILEPOS_PARAMS) {
  if (!hm) {
    return errg(err_invalid_arugment);
  }
  if (hm->ptr) {
#ifdef ALLOCATE_LOGGER
    struct hmap_udata ud = {
        .filepos = filepos,
    };
    hashmap_set_udata(hm->ptr, &ud);
#endif
    hashmap_free(hm->ptr);
    hm->ptr = NULL;
  }
  hm->get_key = NULL;
  return eok();
}
