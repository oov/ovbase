#include "wstr.h"

#ifdef USE_WSTR

error wstr_replace_all_(struct wstr *const ws,
                        wchar_t const *const find,
                        wchar_t const *const replacement MEM_FILEPOS_PARAMS) {
  if (!ws || !find || !replacement) {
    return errg(err_invalid_arugment);
  }
  if (!ws->ptr || !ws->len) {
    return eok();
  }
  ptrdiff_t const findlen = (ptrdiff_t)wcslen(find);
  if (findlen == 0) {
    return eok();
  }
  error err = eok();
  struct wstr tmp = {0};
  ptrdiff_t pos = 0;
  for (;;) {
    wchar_t const *const found = wcsstr(ws->ptr + pos, find);
    if (!found) {
      err = wstr_cat_(&tmp, ws->ptr + pos MEM_FILEPOS_VALUES_PASSTHRU);
      if (efailed(err)) {
        err = ethru(err);
        goto cleanup;
      }
      break;
    }
    ptrdiff_t const foundpos = found - ws->ptr;
    err = wstr_ncat_(&tmp, ws->ptr + pos, (size_t)(foundpos - pos) MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    err = wstr_cat_(&tmp, replacement MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    pos = foundpos + findlen;
  }

  err = wstr_cpy_(ws, tmp.ptr MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  array_free_core_((struct array *)&tmp MEM_FILEPOS_VALUES_PASSTHRU);
  return err;
}

#endif
