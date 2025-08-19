#include "hmap.h"

#include <string.h>

static uint64_t hm_hash_static(void const *const item, uint64_t const seed0, uint64_t const seed1, void *const udata) {
  struct hmap_udata const *const ud = udata;
  return hashmap_sip(item, ud->hm->size, seed0, seed1);
}

static int hm_compare_static(const void *a, const void *b, void *const udata) {
  struct hmap_udata const *const ud = udata;
  return memcmp(a, b, ud->hm->size);
}

error hmap_new_static(struct hmap *const hm,
                      size_t const item_size,
                      size_t const cap,
                      size_t const key_bytes MEM_FILEPOS_PARAMS) {
  if (!hm || hm->ptr) {
    return errg(err_invalid_arugment);
  }
  struct hmap_udata ud = {
      .hm = NULL,
#ifdef ALLOCATE_LOGGER
      .filepos = filepos,
#endif
  };
  uint64_t hash = ov_splitmix64_next(get_global_hint());
  uint64_t const s0 = ov_splitmix64(hash);
  hash = ov_splitmix64_next(hash);
  uint64_t const s1 = ov_splitmix64(hash);
  struct hashmap *const h = hashmap_new_with_allocator(
      hm_realloc, hm_free, item_size, cap, s0, s1, hm_hash_static, hm_compare_static, NULL, &ud);
  if (!h) {
    return errg(err_out_of_memory);
  }
  hm->ptr = h;
  hm->size = key_bytes;
  return eok();
}
