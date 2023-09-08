#include "hmap.h"

static uint64_t hm_hash_dynamic(void const *const item, uint64_t const seed0, uint64_t const seed1, void *const udata) {
  struct hmap_udata const *const ud = udata;
  void const *p = NULL;
  size_t len = 0;
  ud->hm->get_key(item, &p, &len);
  return hashmap_sip(p, len, seed0, seed1);
}

static int hm_compare_dynamic(void const *const a, void const *const b, void *const udata) {
  struct hmap_udata const *const ud = udata;
  void const *p0 = NULL, *p1 = NULL;
  size_t len0 = 0, len1 = 0;
  ud->hm->get_key(a, &p0, &len0);
  ud->hm->get_key(b, &p1, &len1);
  int r = memcmp(p0, p1, len0 < len1 ? len0 : len1);
  if (len0 == len1 || r != 0) {
    return r;
  }
  return len0 < len1 ? -1 : 1;
}

error hmap_new_dynamic(struct hmap *const hm,
                       size_t const item_size,
                       size_t const cap,
                       hm_get_key const get_key MEM_FILEPOS_PARAMS) {
  if (!hm || hm->ptr || !get_key) {
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
      hm_malloc, hm_realloc, hm_free, item_size, cap, s0, s1, hm_hash_dynamic, hm_compare_dynamic, NULL, &ud);
  if (!h) {
    return errg(err_out_of_memory);
  }
  hm->ptr = h;
  hm->get_key = get_key;
  return eok();
}
