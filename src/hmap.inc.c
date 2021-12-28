#include "../include/base.h"

struct hmap_udata {
  struct hmap *hm;
  struct base_filepos const *filepos;
};

static void *hm_malloc(size_t const s, void *const udata) {
#ifdef ALLOCATE_LOGGER
  struct hmap_udata const *const ud = udata;
  struct base_filepos const *const filepos = ud->filepos;
#else
  (void)udata;
#endif
  void *r = NULL;
  if (!mem_core_(&r, s MEM_FILEPOS_VALUES_PASSTHRU)) {
    return NULL;
  }
  return r;
}

static void hm_free(void *p, void *const udata) {
#ifdef ALLOCATE_LOGGER
  struct hmap_udata const *const ud = udata;
  struct base_filepos const *const filepos = ud->filepos;
#else
  (void)udata;
#endif
  mem_core_(&p, 0 MEM_FILEPOS_VALUES_PASSTHRU);
}

static uint64_t hm_hash_static(void const *const item, uint64_t const seed0, uint64_t const seed1, void *const udata) {
  struct hmap_udata const *const ud = udata;
  return hashmap_sip(item, ud->hm->size, seed0, seed1);
}

static int hm_compare_static(const void *a, const void *b, void *const udata) {
  struct hmap_udata const *const ud = udata;
  return memcmp(a, b, ud->hm->size);
}

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
  uint64_t hash = base_splitmix64_next(get_global_hint());
  uint64_t const s0 = base_splitmix64(hash);
  hash = base_splitmix64_next(hash);
  uint64_t const s1 = base_splitmix64(hash);
  struct hashmap *const h = hashmap_new_with_allocator(
      hm_malloc, hm_free, item_size, cap, s0, s1, hm_hash_static, hm_compare_static, NULL, &ud);
  if (!h) {
    return errg(err_out_of_memory);
  }
  hm->ptr = h;
  hm->size = key_bytes;
  return eok();
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
  uint64_t hash = base_splitmix64_next(get_global_hint());
  uint64_t const s0 = base_splitmix64(hash);
  hash = base_splitmix64_next(hash);
  uint64_t const s1 = base_splitmix64(hash);
  struct hashmap *const h = hashmap_new_with_allocator(
      hm_malloc, hm_free, item_size, cap, s0, s1, hm_hash_dynamic, hm_compare_dynamic, NULL, &ud);
  if (!h) {
    return errg(err_out_of_memory);
  }
  hm->ptr = h;
  hm->get_key = get_key;
  return eok();
}

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

error hmap_clear(struct hmap *const hm) {
  if (!hm) {
    return errg(err_invalid_arugment);
  }
  if (hm->ptr) {
    hashmap_clear(hm->ptr, true);
  }
  return eok();
}

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
  *item = hashmap_get(hm->ptr, key_item);
  return eok();
}

error hmap_set(struct hmap *const hm, void const *const item, void **const old_item MEM_FILEPOS_PARAMS) {
  if (!hm || !hm->ptr) {
    return errg(err_invalid_arugment);
  }
  struct hmap_udata ud = {
      .hm = hm,
#ifdef ALLOCATE_LOGGER
      .filepos = filepos,
#endif
  };
  hashmap_set_udata(hm->ptr, &ud);
  void *r = hashmap_set(hm->ptr, item);
  if (r == NULL && hashmap_oom(hm->ptr)) {
    return errg(err_out_of_memory);
  }
  if (old_item) {
    *old_item = r;
  }
  return eok();
}

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

error hmap_scan(struct hmap *const hm, bool (*iter)(void const *const item, void *const udata), void *const udata) {
  if (!hm || !hm->ptr) {
    return errg(err_invalid_arugment);
  }
  if (!hashmap_scan(hm->ptr, iter, udata)) {
    return errg(err_abort);
  }
  return eok();
}
