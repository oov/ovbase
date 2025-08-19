#include "str.h"

#ifndef OV_NOSTR
#  ifdef USE_STR

error str_str_(struct str const *const s, char const *const s2, ptrdiff_t *const pos) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }
  if (!pos) {
    return errg(err_null_pointer);
  }
  if (!s->ptr || !s->len) {
    *pos = -1;
    return eok();
  }
  char const *const found = strstr(s->ptr, s2);
  if (!found) {
    *pos = -1;
    return eok();
  }
  *pos = found - s->ptr;
  return eok();
}

#  endif
#endif
