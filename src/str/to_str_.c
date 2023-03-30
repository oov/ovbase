#include "ovbase.h"

#include "ovutf.h"

#if defined(USE_STR)

NODISCARD error to_str_(wchar_t const *const src, size_t const src_len, struct str *const dest MEM_FILEPOS_PARAMS) {
  size_t sz = ov_wchar_to_utf8_len(src, src_len);
  if (!sz) {
    return errg(err_fail);
  }
  error err = str_grow_(dest, sz + 1 ERR_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  sz = ov_wchar_to_utf8(src, src_len, dest->ptr, sz, NULL);
  if (!sz) {
    return errg(err_fail);
  }
  dest->len = sz;
  return eok();
}

#endif
