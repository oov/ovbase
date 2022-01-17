#include "wstr.h"

#ifdef USE_WSTR

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
  ws->ptr[ws2len] = L'\0';
  ws->len = wcslen(ws->ptr);
  return eok();
}

#endif
