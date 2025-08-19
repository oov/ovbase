#include "str.h"

#ifndef OV_NOSTR
#  ifdef USE_STR

#    include "ovnum.h"

NODISCARD error str_itoa_(int64_t const v, struct str *const dest MEM_FILEPOS_PARAMS) {
  if (!dest) {
    return errg(err_null_pointer);
  }
  char buf[32];
  error err = str_cpy_(dest, ov_itoa(v, buf) MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}

#  endif
#endif
