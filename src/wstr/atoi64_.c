#include "wstr.h"

#ifdef USE_WSTR

NODISCARD error wstr_atoi64_(struct wstr const *const s, int64_t *const dest) {
  if (!s) {
    return errg(err_invalid_arugment);
  }
  if (!dest) {
    return errg(err_null_pointer);
  }

  uint64_t r = 0, pr = 0;
  wchar_t *ptr = s->ptr;
  size_t len = s->len;
  int64_t sign = 1;
  if (len && (*ptr == L'+' || *ptr == L'-')) {
    sign = *ptr == L'-' ? -1 : 1;
    ++ptr;
    --len;
  }
  for (size_t i = 0; i < len; ++i) {
    if (i >= 19 || L'0' > ptr[i] || ptr[i] > L'9') {
      return errg(err_fail);
    }
    pr = r;
    r = r * 10 + (uint64_t)(ptr[i] - L'0');
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
