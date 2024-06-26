#include <ovarray.h>

#include <assert.h>

#include "mem.h"

struct ov_array_header {
  size_t len;
  size_t cap;
};

#define OV_ARRAY_HEADER(a) ((struct ov_array_header *)(a) - 1)
#define OV_ARRAY_HEADER_CONST(a) ((struct ov_array_header const *)(a) - 1)

static inline size_t zumax(size_t const a, size_t const b) { return a > b ? a : b; }

bool ov_array_grow(void **const a, size_t const itemsize, size_t const newcap MEM_FILEPOS_PARAMS) {
  assert(a != NULL);
  assert(itemsize != 0);

  struct ov_array_header *h = *a ? OV_ARRAY_HEADER(*a) : NULL;
  size_t const curcap = h ? h->cap : 0;
  if (newcap <= curcap) {
    return true;
  }
  size_t const cap = zumax(curcap * 2, newcap);
  if (!mem_core_(&h, sizeof(struct ov_array_header) + cap * itemsize MEM_FILEPOS_VALUES_PASSTHRU)) {
    return false;
  }
  h->cap = cap;
  if (curcap == 0) {
    h->len = 0;
  }
  *a = h + 1;
  return true;
}

void ov_array_destroy(void **const a MEM_FILEPOS_PARAMS) {
  assert(a != NULL);
  if (*a == NULL) {
    return;
  }
  struct ov_array_header *h = OV_ARRAY_HEADER(*a);
  mem_core_(&h, 0 MEM_FILEPOS_VALUES_PASSTHRU);
  *a = NULL;
}

size_t ov_array_length(void const *const a) { return a ? OV_ARRAY_HEADER_CONST(a)->len : 0; }

void ov_array_set_length(void *const a, size_t const newlen) {
  if (!a) {
    return;
  }
  OV_ARRAY_HEADER(a)->len = newlen;
}

size_t ov_array_capacity(void const *const a) { return a ? OV_ARRAY_HEADER_CONST(a)->cap : 0; }

bool ov_array_prepare_for_push(void **const a, size_t const itemsize MEM_FILEPOS_PARAMS) {
  size_t const new_len = ov_array_length(*a) + 1;
  if (!ov_array_grow(a, itemsize, new_len MEM_FILEPOS_VALUES_PASSTHRU)) {
    return false;
  }
  ov_array_set_length(*a, new_len);
  return true;
}

size_t ov_array_length_decrement(void *const a) {
  return a && OV_ARRAY_HEADER_CONST(a)->len ? --(OV_ARRAY_HEADER(a)->len) : 0;
}

bool ov_bitarray_grow(ov_bitarray **const a, size_t const newcap MEM_FILEPOS_PARAMS) {
  assert(a != NULL);

  struct ov_array_header *h = *a ? OV_ARRAY_HEADER(*a) : NULL;
  size_t const curcap = h ? h->cap : 0;
  size_t const realnewcap = OV_BITARRAY_LENGTH_TO_BYTES(newcap);
  if (realnewcap <= curcap) {
    return true;
  }
  if (!mem_core_(&h, sizeof(struct ov_array_header) + realnewcap MEM_FILEPOS_VALUES_PASSTHRU)) {
    return false;
  }
  h->cap = realnewcap;
  if (curcap == 0) {
    h->len = 0;
  }
  *a = (void *)(h + 1);
  memset(*a + curcap, 0, (realnewcap - curcap));
  return true;
}

NODISCARD error ov_bitarray_alloc(ov_bitarray **const a, size_t const len MEM_FILEPOS_PARAMS) {
  size_t const n = OV_BITARRAY_LENGTH_TO_BYTES(len);
  error err = mem_(a, n, 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  memset(*a, 0, n);
  return eok();
}
