#include "str.h"

#ifndef OV_NOSTR
#  ifdef USE_STR

error str_ncpy_(struct str *const s, char const *const s2, size_t const s2len MEM_FILEPOS_PARAMS) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }

  error err = str_grow(s, s2len + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  strncpy(s->ptr, s2, s2len);
  s->ptr[s2len] = '\0';
  s->len = strlen(s->ptr);
  return eok();
}

#  endif
#endif
