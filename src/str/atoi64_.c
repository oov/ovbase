#include "str.h"

#ifdef USE_STR

NODISCARD error str_atoi64_(struct str const *const s, int64_t *const dest) {
  if (!s) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  uint64_t r = 0, pr = 0;
  char *ptr = s->ptr;
  size_t len = s->len;
  int64_t sign = 1;
  if (len && (*ptr == '+' || *ptr == '-')) {
    sign = *ptr == '-' ? -1 : 1;
    ++ptr;
    --len;
  }
  for (size_t i = 0; i < len; ++i) {
    if (i >= 19 || '0' > ptr[i] || ptr[i] > '9') {
      return errg(err_fail);
    }
    pr = r;
    r = r * 10 + (uint64_t)(ptr[i] - '0');
    if (r < pr) {
      return errg(err_fail);
    }
    continue;
  }
  if ((r >> 63) && ((sign > 0) || (r << 1))) {
    return errg(err_fail);
  }
  *dest = (int64_t)(r)*sign;
  return eok();
}

#endif
