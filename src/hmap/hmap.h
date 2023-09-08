#pragma once

#include <ovbase.h>

#include "../../3rd/hashmap.c/hashmap.h"

struct hmap_udata {
  struct hmap *hm;
  struct ov_filepos const *filepos;
};

void *hm_malloc(size_t const s, void *const udata);
void *hm_realloc(void *p, size_t const s, void *const udata);
void hm_free(void *p, void *const udata);
