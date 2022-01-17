#include "wstr.h"

#ifdef USE_WSTR

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

#endif
