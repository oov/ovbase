#include "def.h"

#include <assert.h>

bool FUNCNAME(atou)(CHAR_TYPE const *const ptr, UINT_TYPE *const dest, bool const strict) {
  assert(ptr != NULL && "ptr must not be NULL");
  assert(dest != NULL && "dest must not be NULL");
  if (!ptr || !dest) {
    return false;
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
  *dest = r;
  return true;
}
