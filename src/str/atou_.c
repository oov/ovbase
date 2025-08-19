#include "str.h"

#ifndef OV_NOSTR
#  ifdef USE_STR

#    include "ovnum.h"

NODISCARD error str_atou_(struct str const *const s, uint64_t *const dest) {
  if (!s) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }
  if (!ov_atou(s->ptr, dest, true)) {
    return errg(err_fail);
  }
  return eok();
}

#  endif
#endif
