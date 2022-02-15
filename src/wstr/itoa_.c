#include "wstr.h"

#ifdef USE_WSTR

#  include "ovnum.h"

NODISCARD error wstr_itoa_(int64_t v, struct wstr *const dest MEM_FILEPOS_PARAMS) {
  if (!dest) {
    return errg(err_null_pointer);
  }
  wchar_t buf[32];
  error err = wstr_cpy_(dest, ov_itoa(v, buf) MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}

#endif
