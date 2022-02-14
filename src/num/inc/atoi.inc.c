#include "def.h"

bool FUNCNAME(atoi)(CHAR_TYPE const *ptr, INT_TYPE *const dest, bool const strict) {
  if (!ptr || !dest) {
    return false;
  }
  INT_TYPE const sign = *ptr == STR('-') ? -1 : 1;
  if ((sign == -1) || (*ptr == STR('+'))) {
    ++ptr;
  }
  UINT_TYPE r = 0, pr;
  CHAR_TYPE const *p = ptr;
  while (STR('0') <= *p && *p <= STR('9')) {
    pr = r;
    r = r * 10 + (UINT_TYPE)(*p++ - STR('0'));
    if (r < pr) {
      return false;
    }
  }
  if (ptr == p) {
    return false;
  }
  if (strict && *p != STR('\0')) {
    return false;
  }
  if ((r >> (sizeof(r) * 8 - 1)) && ((sign > 0) || (r << 1))) {
    return false;
  }
  *dest = (INT_TYPE)(r)*sign;
  return true;
}
