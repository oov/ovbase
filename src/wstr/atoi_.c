#include "wstr.h"

#ifdef USE_WSTR

#  include "ovnum.h"

NODISCARD error wstr_atoi_(struct wstr const *const s, int64_t *const dest) {
  if (!s) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }
  if (!ov_atoi(s->ptr, dest, true)) {
    return errg(err_fail);
  }
  return eok();
}

#endif
