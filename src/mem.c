#include "mem.h"

error mem_(void *const pp, size_t const n, size_t const item_size MEM_FILEPOS_PARAMS) {
  if (!pp || !item_size) {
    return errg(err_invalid_arugment);
  }
  if (!mem_core_(pp, n * item_size MEM_FILEPOS_VALUES_PASSTHRU)) {
    return errg(err_out_of_memory);
  }
  return eok();
}

void mem_free_(void *const pp MEM_FILEPOS_PARAMS) {
  if (!pp) {
    return;
  }
  mem_core_(pp, 0 MEM_FILEPOS_VALUES_PASSTHRU);
  return;
}
