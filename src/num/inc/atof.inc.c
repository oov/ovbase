#include "def.h"

bool FUNCNAME(atof)(CHAR_TYPE const *ptr, FLOAT_TYPE *const dest, bool const strict) {
  if (!ptr || !dest) {
    return false;
  }
  INT_TYPE const sign = *ptr == STR('-') ? -1 : 1;
  if ((sign == -1) || (*ptr == STR('+'))) {
    ++ptr;
  }
  INT_TYPE i = 0;
  INT_TYPE f = 0;
  INT_TYPE d = 0;
  INT_TYPE prev = 0;
  CHAR_TYPE const *p = ptr;
  while ((STR('0') <= *p && *p <= STR('9')) || *p == STR('.')) {
    if (*p == STR('.')) {
      if (d != 0) {
        return false;
      }
      d = 1;
      ++p;
      continue;
    }
    if (d != 0) {
      prev = f;
      f = f * 10 + (INT_TYPE)(*p++ - STR('0'));
      if (prev > f) {
        return false;
      }
      d *= 10;
    } else {
      prev = i;
      i = i * 10 + (INT_TYPE)(*p++ - STR('0'));
      if (prev > i) {
        return false;
      }
    }
  }
  if (ptr == p) {
    return false;
  }
  if (strict && *p != STR('\0')) {
    return false;
  }
  if (d != 0) {
    *dest = ((FLOAT_TYPE)(i) + ((FLOAT_TYPE)(f) / (FLOAT_TYPE)(d))) * (FLOAT_TYPE)(sign);
  } else {
    *dest = (FLOAT_TYPE)(i) * (FLOAT_TYPE)(sign);
  }
  return true;
}
