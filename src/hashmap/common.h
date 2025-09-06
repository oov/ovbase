#pragma once

#include <ovbase.h>
#include <ovhashmap.h>

#include "../../3rd/hashmap.c/hashmap.h"

struct ov_hashmap {
  struct hashmap *map;
  union {
    ov_hashmap_get_key_func get_key;
    size_t key_bytes;
  };
#ifdef ALLOCATE_LOGGER
  struct ov_filepos const *filepos;
#endif
};

uint64_t sip_hash_1_3(const void *data, size_t len, uint64_t seed0, uint64_t seed1);
void *ov_hm_realloc(void *const p, size_t const s, void *const udata);
void ov_hm_free(void *const p, void *const udata);
