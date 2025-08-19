#pragma once

#include <ovbase.h>

#ifndef OV_NOSTR
#  ifdef USE_STR
NODISCARD static inline error str_grow(struct str *const s, size_t const least_size MEM_FILEPOS_PARAMS) {
  return array_grow_((struct array *)s, sizeof(char), least_size MEM_FILEPOS_VALUES_PASSTHRU);
}
#  endif
#endif
