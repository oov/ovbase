#include "str.h"

#ifndef OV_NOSTR
#  ifdef USE_STR

error str_cpy_(struct str *const s, char const *const s2 MEM_FILEPOS_PARAMS) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }

  size_t const s2len = strlen(s2);
  error err = str_grow(s, s2len + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  strcpy(s->ptr, s2);
  s->len = s2len;
  return eok();
}

#  endif
#endif
