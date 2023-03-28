#include "ovbase.h"

#include "ovutf.h"

#if defined(USE_STR)

NODISCARD error to_str_(wchar_t const *const src, size_t const src_len, struct str *const dest MEM_FILEPOS_PARAMS) {
  size_t sz = 0;
  if (!wchar_to_utf8_len(src, src_len, &sz)) {
    return emsg(err_type_generic, err_fail, &native_unmanaged_const(NSTR("failed to convert string")));
  }
  error err = str_grow_(dest, sz + 1 ERR_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  wchar_to_utf8(src, src_len, dest->ptr, sz, NULL, NULL);
  dest->ptr[sz] = L'\0';
  dest->len = sz;
cleanup:
  return err;
}

#endif
