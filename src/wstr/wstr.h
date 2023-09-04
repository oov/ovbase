#pragma once

#include <ovbase.h>

#ifdef USE_WSTR
NODISCARD static inline error wstr_grow(struct wstr *const ws, size_t const least_size MEM_FILEPOS_PARAMS) {
  return array_grow_((struct array *)ws, sizeof(wchar_t), least_size MEM_FILEPOS_VALUES_PASSTHRU);
}
#endif
