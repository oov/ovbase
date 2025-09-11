#include "common.h"

#include <ovrand.h>
#include <string.h>

static uint64_t calc_hash(void const *const item, uint64_t const seed0, uint64_t const seed1, void const *const udata) {
  struct ov_hashmap const *const hm = (struct ov_hashmap const *)udata;
  if (!item || !hm || !hm->get_key) {
    return 0;
  }
  void const *p = NULL;
  size_t len = 0;
  hm->get_key(item, &p, &len);
  return sip_hash_1_3(p, len, seed0, seed1);
}

static int compare(void const *const a, void const *const b, void const *const udata) {
  struct ov_hashmap const *const hm = (struct ov_hashmap const *)udata;
  if (!hm || !hm->get_key) {
    return 0;
  }
  if (!a && !b) {
    return 0;
  }
  if (!a) {
    return -1;
  }
  if (!b) {
    return 1;
  }
  void const *p0 = NULL, *p1 = NULL;
  size_t len0 = 0, len1 = 0;
  hm->get_key(a, &p0, &len0);
  hm->get_key(b, &p1, &len1);
  int r = memcmp(p0, p1, len0 < len1 ? len0 : len1);
  if (len0 == len1 || r != 0) {
    return r;
  }
  return len0 < len1 ? -1 : 1;
}

struct ov_hashmap *ov_hashmap_create_dynamic(size_t const item_size,
                                             size_t const cap,
                                             ov_hashmap_get_key_func const get_key MEM_FILEPOS_PARAMS) {
  if (!get_key) {
    return NULL;
  }

  struct ov_hashmap *result = NULL;
  struct ov_hashmap *hm = NULL;

  if (!ov_mem_realloc(&hm, 1, sizeof(*hm), NULL MEM_FILEPOS_VALUES_PASSTHRU)) {
    goto cleanup;
  }

  *hm = (struct ov_hashmap){
      .get_key = get_key,
  };

  {
    uint64_t hash = ov_rand_splitmix64_next(ov_rand_get_global_hint());
    uint64_t const s0 = ov_rand_splitmix64(hash);
    hash = ov_rand_splitmix64_next(hash);
    uint64_t const s1 = ov_rand_splitmix64(hash);

#ifdef ALLOCATE_LOGGER
    hm->filepos = filepos;
#endif
    hm->map =
        hashmap_new_with_allocator(ov_hm_realloc, ov_hm_free, item_size, cap, s0, s1, calc_hash, compare, NULL, hm);
    if (!hm->map) {
      goto cleanup;
    }
  }

  result = hm;
  hm = NULL;

cleanup:
  if (hm) {
    if (hm->map) {
      hashmap_free(hm->map);
      hm->map = NULL;
    }
    ov_mem_free(&hm MEM_FILEPOS_VALUES_PASSTHRU);
  }
  return result;
}
