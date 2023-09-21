#include "mem.h"

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

NODISCARD error mem_aligned_alloc_(void *const pp,
                                   size_t const n,
                                   size_t const item_size,
                                   size_t const align MEM_FILEPOS_PARAMS) {
  if (!pp || !n || !item_size || align > 256) {
    return errg(err_invalid_arugment);
  }
  if (*(void **)pp != NULL) {
    return errg(err_invalid_arugment);
  }
  *(void **)pp = mi_malloc_aligned(n * item_size, align);
  if (*(void **)pp == NULL) {
    return errg(err_out_of_memory);
  }
#  if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
  mem_log_allocated(*(void **)pp MEM_FILEPOS_VALUES_PASSTHRU);
#  endif
  return eok();
}

NODISCARD error mem_aligned_free_(void *const pp MEM_FILEPOS_PARAMS) {
  if (!pp) {
    return errg(err_invalid_arugment);
  }
  if (*(void **)pp == NULL) {
    return errg(err_invalid_arugment);
  }
  mi_free(*(void **)pp);
#  if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
  mem_log_free(*(void **)pp);
#  endif
  *(void **)pp = NULL;
#  ifdef ALLOCATE_LOGGER
  (void)filepos;
#  endif
  return eok();
}

#else

NODISCARD error mem_aligned_alloc_(void *const pp,
                                   size_t const n,
                                   size_t const item_size,
                                   size_t const align MEM_FILEPOS_PARAMS) {
  if (!pp || !n || !item_size || align > 256) {
    return errg(err_invalid_arugment);
  }
  if (*(void **)pp != NULL) {
    return errg(err_invalid_arugment);
  }
  uint8_t *p = NULL;
  if (!mem_core_(&p, n * item_size + align MEM_FILEPOS_VALUES_PASSTHRU)) {
    return errg(err_out_of_memory);
  }
  size_t const offset = align - (((size_t)p) % align);
  *(void **)pp = p + offset;
  *(p + offset - 1) = (uint8_t)(offset - 1);
  return eok();
}

NODISCARD error mem_aligned_free_(void *const pp MEM_FILEPOS_PARAMS) {
  if (!pp) {
    return errg(err_invalid_arugment);
  }
  if (*(void **)pp == NULL) {
    return errg(err_invalid_arugment);
  }
  uint8_t *p = *(void **)pp;
  size_t offset = (size_t)(*(p - 1));
  p -= offset + 1;
  mem_core_(&p, 0 MEM_FILEPOS_VALUES_PASSTHRU);
  *(void **)pp = NULL;
  return eok();
}

#endif
