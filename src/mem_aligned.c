#include "mem.h"
#include <assert.h>

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
                          size_t const align MEM_FILEPOS_PARAMS) {
  assert(pp != NULL && "pp must not be NULL");
  assert(n > 0 && "n must be greater than 0");
  assert(item_size > 0 && "item_size must be greater than 0");
  assert(align > 0 && align <= 256 && (align & (align - 1)) == 0 &&
         "align must be a power of 2, greater than 0 and at most 256");
#  ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
#  endif
  if (!pp || !n || !item_size || align == 0 || align > 256 || (align & (align - 1)) != 0) {
    return false;
  }
  if (*(void **)pp != NULL) {
    return false; // Double allocation not allowed
  }
  *(void **)pp = mi_malloc_aligned(n * item_size, align);
  if (*(void **)pp == NULL) {
    return false;
  }
#  if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
  mem_log_allocated(*(void **)pp MEM_FILEPOS_VALUES_PASSTHRU);
#  endif
  return true;
}

void ov_mem_aligned_free(void *const pp MEM_FILEPOS_PARAMS) {
  assert(pp != NULL && "pp must not be NULL");
#  ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
#  endif
  if (!pp) {
    return;
  }
  if (*(void **)pp == NULL) {
    return;
  }
  mi_free(*(void **)pp);
#  if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
  mem_log_free(*(void **)pp MEM_FILEPOS_VALUES_PASSTHRU);
#  endif
  *(void **)pp = NULL;
}

#else

bool ov_mem_aligned_alloc(void *const pp,
                          size_t const n,
                          size_t const item_size,
                          size_t const align MEM_FILEPOS_PARAMS) {
  assert(pp != NULL && "pp must not be NULL");
  assert(n > 0 && "n must be greater than 0");
  assert(item_size > 0 && "item_size must be greater than 0");
  assert(align > 0 && align <= 256 && (align & (align - 1)) == 0 &&
         "align must be a power of 2, greater than 0 and at most 256");
#  ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
#  endif
  if (!pp || !n || !item_size || align == 0 || align > 256 || (align & (align - 1)) != 0) {
    return false;
  }
  if (*(void **)pp != NULL) {
    return false; // Double allocation not allowed
  }
  uint8_t *p = NULL;
  if (!mem_core_(&p, n * item_size + align MEM_FILEPOS_VALUES_PASSTHRU)) {
    return false;
  }
  size_t const offset = align - (((size_t)p) % align);
  *(void **)pp = p + offset;
  *(p + offset - 1) = (uint8_t)(offset - 1);
  return true;
}

void ov_mem_aligned_free(void *const pp MEM_FILEPOS_PARAMS) {
  assert(pp != NULL && "pp must not be NULL");
#  ifdef ALLOCATE_LOGGER
  assert(filepos != NULL && "filepos must not be NULL");
#  endif
  if (!pp) {
    return;
  }
  if (*(void **)pp == NULL) {
    return;
  }
  uint8_t *p = *(uint8_t **)pp;
  size_t offset = (size_t)(*(p - 1));
  p -= offset + 1;
  mem_core_(&p, 0 MEM_FILEPOS_VALUES_PASSTHRU);
  *(void **)pp = NULL;
}

#endif
