#include "base.h"

bool array_grow_core_(struct array *const p, size_t const elem_size, size_t const least_size MEM_FILEPOS_PARAMS) {
  if (p->cap >= least_size) {
    return true;
  }
  static size_t const block_size = 8;
  size_t const newcap = (least_size + block_size - 1) & ~(block_size - 1);
  if (!mem_core_(&p->ptr, newcap * elem_size MEM_FILEPOS_VALUES_PASSTHRU)) {
    return false;
  }
  memset((char *)(p->ptr) + p->cap * elem_size, 0, (newcap - p->cap) * elem_size);
  p->cap = newcap;
  return true;
}

error array_grow_(struct array *const p, size_t const elem_size, size_t const least_size MEM_FILEPOS_PARAMS) {
  if (!p) {
    return errg(err_invalid_arugment);
  }
  if (p->ptr && p->cap == 0) {
    return emsg(err_type_generic, err_unexpected, &native_unmanaged(NSTR("Unmanaged pointer cannot be grown.")));
  }
  return array_grow_core_(p, elem_size, least_size MEM_FILEPOS_VALUES_PASSTHRU) ? eok() : errg(err_out_of_memory);
}

void array_free_core_(struct array *const p MEM_FILEPOS_PARAMS) {
  if (p->ptr) {
    if (p->cap == 0) {
      p->ptr = NULL; // unmanaged pointer
    } else {
      mem_core_(&p->ptr, 0 MEM_FILEPOS_VALUES_PASSTHRU);
    }
  }
  p->len = 0;
  p->cap = 0;
}

error array_free_(struct array *const p MEM_FILEPOS_PARAMS) {
  if (!p) {
    return errg(err_invalid_arugment);
  }
  array_free_core_(p MEM_FILEPOS_VALUES_PASSTHRU);
  return eok();
}
