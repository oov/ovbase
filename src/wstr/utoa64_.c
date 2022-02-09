#include "wstr.h"

#ifdef USE_WSTR

NODISCARD error wstr_utoa64_(uint64_t v, struct wstr *const dest MEM_FILEPOS_PARAMS) {
  if (!dest) {
    return errg(err_null_pointer);
  }

  enum {
    buf_size = 32,
  };
  wchar_t buf[buf_size] = {0};
  size_t i = buf_size - 1;
  while (v > 9) {
    buf[i--] = (v % 10) + L'0';
    v /= 10;
  }
  buf[i] = (wchar_t)v + L'0';
  error err = wstr_ncpy_(dest, buf + i, buf_size - i MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}

#endif
