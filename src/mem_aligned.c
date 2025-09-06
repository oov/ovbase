#include "mem.h"

// New error system versions

#ifdef USE_MIMALLOC

#  ifdef __GNUC__
#    pragma GCC diagnostic push
#    if __has_warning("-Wreserved-identifier")
#      pragma GCC diagnostic ignored "-Wreserved-identifier"
#    endif
#  endif // __GNUC__
#  include <mimalloc.h>
#  ifdef __GNUC__
#    pragma GCC diagnostic pop
#  endif // __GNUC__

bool ov_mem_aligned_alloc(void *const pp,
                          size_t const n,
                          size_t const item_size,
                          size_t const align,
                          struct ov_error *const err MEM_FILEPOS_PARAMS) {
  if (!pp || !n || !item_size || align > 256) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }
  if (*(void **)pp != NULL) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }
  *(void **)pp = mi_malloc_aligned(n * item_size, align);
  if (*(void **)pp == NULL) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
    return false;
  }
#  if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
  mem_log_allocated(*(void **)pp MEM_FILEPOS_VALUES_PASSTHRU);
#  endif
  return true;
}

void ov_mem_aligned_free(void *const pp MEM_FILEPOS_PARAMS) {
  if (!pp || *(void **)pp == NULL) {
    return;
  }
  mi_free(*(void **)pp);
#  if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
  mem_log_free(*(void **)pp);
#  endif
  *(void **)pp = NULL;
#  ifdef ALLOCATE_LOGGER
  (void)filepos;
#  endif
}

#else

bool ov_mem_aligned_alloc(void *const pp,
                          size_t const n,
                          size_t const item_size,
                          size_t const align,
                          struct ov_error *const err MEM_FILEPOS_PARAMS) {
  if (!pp || !n || !item_size || align > 256) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }
  if (*(void **)pp != NULL) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }
  uint8_t *p = NULL;
  if (!mem_core_(&p, n * item_size + align MEM_FILEPOS_VALUES_PASSTHRU)) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
    return false;
  }
  size_t const offset = align - (((size_t)p) % align);
  *(void **)pp = p + offset;
  *(p + offset - 1) = (uint8_t)(offset - 1);
  return true;
}

void ov_mem_aligned_free(void *const pp MEM_FILEPOS_PARAMS) {
  if (!pp || *(void **)pp == NULL) {
    return;
  }
  uint8_t *p = *(uint8_t **)pp;
  size_t offset = (size_t)(*(p - 1));
  p -= offset + 1;
  mem_core_(&p, 0 MEM_FILEPOS_VALUES_PASSTHRU);
  *(void **)pp = NULL;
}

#endif
