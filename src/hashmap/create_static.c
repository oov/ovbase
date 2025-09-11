#include "common.h"

#include <ovrand.h>
#include <string.h>

static uint64_t calc_hash(void const *const item, uint64_t const seed0, uint64_t const seed1, void const *const udata) {
  struct ov_hashmap const *const hm = (struct ov_hashmap const *)udata;
  if (!item || !hm) {
    return 0;
  }
  return sip_hash_1_3(item, hm->key_bytes, seed0, seed1);
}

static int compare(void const *const a, void const *const b, void const *const udata) {
  struct ov_hashmap const *const hm = (struct ov_hashmap const *)udata;
  if (!hm) {
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
  return memcmp(a, b, hm->key_bytes);
}

struct ov_hashmap *
ov_hashmap_create_static(size_t const item_size, size_t const cap, size_t const key_bytes MEM_FILEPOS_PARAMS) {
  if (key_bytes == 0) {
    return NULL;
  }

  struct ov_hashmap *result = NULL;
  struct ov_hashmap *hm = NULL;

  if (!ov_mem_realloc(&hm, 1, sizeof(*hm), NULL MEM_FILEPOS_VALUES_PASSTHRU)) {
    goto cleanup;
  }

  *hm = (struct ov_hashmap){
      .key_bytes = key_bytes,
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
