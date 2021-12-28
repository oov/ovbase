#include "base.h"

#ifdef USE_WSTR

NODISCARD static inline error wstr_grow(struct wstr *const ws, size_t const least_size MEM_FILEPOS_PARAMS) {
  return array_grow_((struct array *)ws, sizeof(wchar_t), least_size MEM_FILEPOS_VALUES_PASSTHRU);
}

error wstr_cpy_(struct wstr *const ws, wchar_t const *const ws2 MEM_FILEPOS_PARAMS) {
  if (!ws || !ws2) {
    return errg(err_invalid_arugment);
  }

  size_t const ws2len = wcslen(ws2);
  error err = wstr_grow(ws, ws2len + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  wcscpy(ws->ptr, ws2);
  ws->len = ws2len;
  return eok();
}

error wstr_cpy_m_(struct wstr *const ws, wchar_t const *const *const ws2 MEM_FILEPOS_PARAMS) {
  if (!ws || !ws2) {
    return errg(err_invalid_arugment);
  }
  error err = eok();
  for (size_t i = 0; ws2[i] != NULL; ++i) {
    err = (i == 0 ? wstr_cpy_ : wstr_cat_)(ws, ws2[i] MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(err)) {
      err = ethru(err);
      return err;
    }
  }
  return err;
}

error wstr_ncpy_(struct wstr *const ws, wchar_t const *const ws2, size_t const ws2len MEM_FILEPOS_PARAMS) {
  if (!ws || !ws2) {
    return errg(err_invalid_arugment);
  }

  error err = wstr_grow(ws, ws2len + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  wcsncpy(ws->ptr, ws2, ws2len);
  ws->len = wcslen(ws->ptr);
  return eok();
}

error wstr_cat_(struct wstr *const ws, wchar_t const *const ws2 MEM_FILEPOS_PARAMS) {
  if (!ws || !ws2) {
    return errg(err_invalid_arugment);
  }

  size_t const ws2len = wcslen(ws2);
  error err = wstr_grow(ws, ws->len + ws2len + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  wcscat(ws->ptr, ws2);
  ws->len += ws2len;
  return eok();
}

error wstr_cat_m_(struct wstr *const ws, wchar_t const *const *const ws2 MEM_FILEPOS_PARAMS) {
  if (!ws || !ws2) {
    return errg(err_invalid_arugment);
  }
  error err = eok();
  for (size_t i = 0; ws2[i] != NULL; ++i) {
    err = wstr_cat_(ws, ws2[i] MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(err)) {
      err = ethru(err);
      return err;
    }
  }
  return err;
}

error wstr_ncat_(struct wstr *const ws, wchar_t const *const ws2, size_t const ws2len MEM_FILEPOS_PARAMS) {
  if (!ws || !ws2) {
    return errg(err_invalid_arugment);
  }

  error err = wstr_grow(ws, ws->len + ws2len + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  wcsncat(ws->ptr, ws2, ws2len);
  ws->len += wcslen(ws->ptr + ws->len);
  return eok();
}

error wstr_str_(struct wstr const *const ws, wchar_t const *const ws2, ptrdiff_t *const pos) {
  if (!ws || !ws2) {
    return errg(err_invalid_arugment);
  }
  if (!pos) {
    return errg(err_null_pointer);
  }
  wchar_t const *const found = wcsstr(ws->ptr, ws2);
  if (!found) {
    *pos = -1;
    return eok();
  }
  *pos = found - ws->ptr;
  return eok();
}

error wstr_replace_all_(struct wstr *const ws,
                        wchar_t const *const find,
                        wchar_t const *const replacement MEM_FILEPOS_PARAMS) {
  if (!ws || !find || !replacement) {
    return errg(err_invalid_arugment);
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

#endif // USE_WSTR
