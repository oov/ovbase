#include <ovmo.h>

#include <ovprintf.h>
#include <ovutf.h>

#ifdef USE_WSTR
int mo_sprintf_wchar(
    wchar_t *const buf, size_t const buflen, wchar_t const *const reference, char const *const format, ...) {
  if (!buf || !buflen || !format) {
    return 0;
  }
  struct wstr ws = {0};
  error err = to_wstr(format, strlen(format), &ws);
  if (efailed(err)) {
    err = ethru(err);
    ereport(err);
    buf[0] = L'\0';
    return 0;
  }
  va_list valist;
  va_start(valist, format);
  int const r = ov_vsnprintf(buf, buflen, reference, ws.ptr, valist);
  va_end(valist);
  ereport(sfree(&ws));
  return r;
}

NODISCARD error mo_vsprintf_wstr(struct wstr *const dest,
                                 wchar_t const *const reference,
                                 char const *const format,
                                 va_list valist) {
  if (!dest || !format) {
    return errg(err_invalid_arugment);
  }
  struct wstr ws = {0};
  error err = to_wstr(format, strlen(format), &ws);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = svsprintf(dest, reference, ws.ptr, valist);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  ereport(sfree(&ws));
  return err;
}

NODISCARD error mo_sprintf_wstr(struct wstr *const dest,
                                wchar_t const *const reference,
                                char const *const format,
                                ...) {
  va_list valist;
  va_start(valist, format);
  error err = mo_vsprintf_wstr(dest, reference, format, valist);
  va_end(valist);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  return err;
}
#endif
