#pragma once

#include <ovbase.h>

#include "../../3rd/hashmap.c/hashmap.h"

struct hmap_udata {
  struct hmap const *hm;
  struct ov_filepos const *filepos;
};

void *hm_malloc(size_t const s, void *const udata);
void *hm_realloc(void *p, size_t const s, void *const udata);
void hm_free(void *p, void *const udata);

uint64_t sip_hash_1_3(const void *data, size_t len, uint64_t seed0, uint64_t seed1);
