#include <ovmo.h>

#include <string.h>

#include <ovprintf.h>
#include <ovutf.h>

static bool convert_format(char const *const format, wchar_t **const ws, struct ov_error *const err) {
  if (!format || !ws) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }
  size_t const formatlen = strlen(format);
  size_t const wslen = ov_utf8_to_wchar_len(format, formatlen) + 1;
  if (!OV_REALLOC(ws, wslen, sizeof(wchar_t), err)) {
    OV_ERROR_TRACE(err);
    return false;
  }
  ov_utf8_to_wchar(format, formatlen, *ws, wslen, NULL);
  return true;
}

int mo_vsnprintf_wchar(
    wchar_t *const buf, size_t const buflen, wchar_t const *const reference, char const *const format, va_list valist) {
  if (!buf || !buflen || !format) {
    return 0;
  }
  wchar_t *ws = NULL;
  int r = 0;
  if (!convert_format(format, &ws, NULL)) {
    buf[0] = L'\0';
    goto cleanup;
  }
  r = ov_vsnprintf(buf, buflen, reference, ws, valist);
cleanup:
  if (ws) {
    OV_FREE(&ws);
  }
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
  int r = 0;
  if (!convert_format(format, &ws, NULL)) {
    goto cleanup;
  }
  r = ov_vpprintf(putc, ctx, reference, ws, valist);
cleanup:
  if (ws) {
    OV_FREE(&ws);
  }
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
