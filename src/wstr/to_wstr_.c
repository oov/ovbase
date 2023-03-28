#include "ovbase.h"

#include "ovutf.h"

#if defined(USE_WSTR)

NODISCARD error to_wstr_(char const *const src, size_t const src_len, struct wstr *const dest MEM_FILEPOS_PARAMS) {
  size_t sz = 0;
  if (!utf8_to_wchar_len(src, src_len, &sz)) {
    return emsg(err_type_generic, err_fail, &native_unmanaged_const(NSTR("failed to convert string")));
  }
  error err = wstr_grow_(dest, sz + 1 ERR_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  utf8_to_wchar(src, src_len, dest->ptr, sz, NULL, NULL);
  dest->ptr[sz] = L'\0';
  dest->len = sz;
cleanup:
  return err;
}

#endif
