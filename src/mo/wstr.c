#include <ovmo.h>

#include <ovprintf.h>
#include <ovutf.h>

static NODISCARD error convert_format(char const *const format, wchar_t **const ws) {
  if (!format || !ws) {
    return errg(err_invalid_arugment);
  }
  size_t const formatlen = strlen(format);
  size_t const wslen = ov_utf8_to_wchar_len(format, formatlen) + 1;
  error err = mem(ws, wslen, sizeof(wchar_t));
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  ov_utf8_to_wchar(format, formatlen, *ws, wslen, NULL);
  return eok();
}

int mo_vsnprintf_wchar(
    wchar_t *const buf, size_t const buflen, wchar_t const *const reference, char const *const format, va_list valist) {
  if (!buf || !buflen || !format) {
    return 0;
  }
  wchar_t *ws = NULL;
  error err = convert_format(format, &ws);
  if (efailed(err)) {
    err = ethru(err);
    ereport(err);
    buf[0] = L'\0';
    return 0;
  }
  int const r = ov_vsnprintf(buf, buflen, reference, ws, valist);
  ereport(mem_free(&ws));
  return r;
}

int mo_snprintf_wchar(
    wchar_t *const buf, size_t const buflen, wchar_t const *const reference, char const *const format, ...) {
  if (!buf || !buflen || !format) {
    return 0;
  }
  va_list valist;
  va_start(valist, format);
  int const r = mo_vsnprintf_wchar(buf, buflen, reference, format, valist);
  va_end(valist);
  return r;
}

int mo_vpprintf_wchar(void (*putc)(int c, void *ctx),
                      void *ctx,
                      wchar_t const *const reference,
                      char const *const format,
                      va_list valist) {
  wchar_t *ws = NULL;
  error err = convert_format(format, &ws);
  if (efailed(err)) {
    err = ethru(err);
    ereport(err);
    return 0;
  }
  int const r = ov_vpprintf(putc, ctx, reference, ws, valist);
  ereport(mem_free(&ws));
  return r;
}

int mo_pprintf_wchar(
    void (*putc)(int c, void *ctx), void *ctx, wchar_t const *const reference, char const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  int const r = mo_vpprintf_wchar(putc, ctx, reference, format, valist);
  va_end(valist);
  return r;
}

#ifndef OV_NOSTR
#  ifdef USE_WSTR

NODISCARD error mo_vsprintf_wstr(struct wstr *const dest,
                                 wchar_t const *const reference,
                                 char const *const format,
                                 va_list valist) {
  if (!dest || !format) {
    return errg(err_invalid_arugment);
  }
  wchar_t *ws = NULL;
  error err = convert_format(format, &ws);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = svsprintf(dest, reference, ws, valist);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  ereport(mem_free(&ws));
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
#  endif
#endif
