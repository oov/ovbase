#include "str.h"

#ifdef USE_STR

NODISCARD error str_itoa64_(int64_t v, struct str *const dest MEM_FILEPOS_PARAMS) {
  if (!dest) {
    return errg(err_null_pointer);
  }
  enum {
    buf_size = 32,
  };
  char buf[buf_size] = {0};
  size_t i = buf_size - 1;
  bool const minus = v < 0;
  uint64_t w = (uint64_t)(minus ? -v : v);
  while (w > 9) {
    buf[i--] = (w % 10) + '0';
    w /= 10;
  }
  buf[i] = (char)w + '0';
  if (minus) {
    buf[--i] = '-';
  }
  error err = str_ncpy_(dest, buf + i, buf_size - i MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}

#endif
