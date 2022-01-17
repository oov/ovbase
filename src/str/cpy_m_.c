#include "str.h"

#ifdef USE_STR

error str_cpy_m_(struct str *const s, char const *const *const s2 MEM_FILEPOS_PARAMS) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }
  error err = eok();
  for (size_t i = 0; s2[i] != NULL; ++i) {
    err = (i == 0 ? str_cpy_ : str_cat_)(s, s2[i] MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(err)) {
      err = ethru(err);
      return err;
    }
  }
  return err;
}

#endif
