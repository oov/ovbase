#include "str.h"

#ifdef USE_STR

#  include "ovnum.h"

NODISCARD error str_atou_(struct str const *const s, uint64_t *const dest) {
  if (!s) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }
  if (!ovbase_atou_char(s->ptr, dest, true)) {
    return errg(err_fail);
  }
  return eok();
}

#endif
