#include "wstr.h"

#ifdef USE_WSTR

error wstr_str_(struct wstr const *const ws, wchar_t const *const ws2, ptrdiff_t *const pos) {
  if (!ws || !ws2) {
    return errg(err_invalid_arugment);
  }
  if (!pos) {
    return errg(err_null_pointer);
  }
  if (!ws->ptr || !ws->len) {
    *pos = -1;
    return eok();
  }
  wchar_t const *const found = wcsstr(ws->ptr, ws2);
  if (!found) {
    *pos = -1;
    return eok();
  }
  *pos = found - ws->ptr;
  return eok();
}

#endif
