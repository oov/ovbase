#include <ovbase.h>

#include <ovutf.h>

#ifndef OV_NOSTR
#  ifdef USE_WSTR

NODISCARD error to_wstr_(char const *const src, size_t const src_len, struct wstr *const dest MEM_FILEPOS_PARAMS) {
  if (!src || !src_len || !dest) {
    return errg(err_invalid_arugment);
  }
  size_t r = ov_utf8_to_wchar_len(src, src_len);
  if (!r) {
    return errg(err_fail);
  }
  error err = wstr_grow_(dest, r + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  r = ov_utf8_to_wchar(src, src_len, dest->ptr, r + 1, NULL);
  if (!r) {
    return errg(err_fail);
  }
  dest->len = r;
  return eok();
}

#  endif
#endif
