#include "wstr.h"

#ifdef USE_WSTR

error wstr_cpy_m_(struct wstr *const ws, wchar_t const *const *const ws2 MEM_FILEPOS_PARAMS) {
  if (!ws || !ws2) {
    return errg(err_invalid_arugment);
  }
  error err = eok();
  for (size_t i = 0; ws2[i] != NULL; ++i) {
    err = (i == 0 ? wstr_cpy_ : wstr_cat_)(ws, ws2[i] MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(err)) {
      err = ethru(err);
      return err;
    }
  }
  return err;
}

#endif
