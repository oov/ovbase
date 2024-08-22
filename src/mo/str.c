#include <ovmo.h>

#include <ovprintf.h>

int mo_vsnprintf_char(
    char *const buf, size_t const buflen, char const *const reference, char const *const format, va_list valist) {
  return ov_vsnprintf(buf, buflen, reference, format, valist);
}

int mo_snprintf_char(char *const buf, size_t const buflen, char const *const reference, char const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  int const r = mo_vsnprintf_char(buf, buflen, reference, format, valist);
  va_end(valist);
  return r;
}

int mo_vpprintf_char(
    void (*putc)(int c, void *ctx), void *ctx, char const *const reference, char const *const format, va_list valist) {
  return ov_vpprintf(putc, ctx, reference, format, valist);
}

int mo_pprintf_char(
    void (*putc)(int c, void *ctx), void *ctx, char const *const reference, char const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  int const r = mo_vpprintf_char(putc, ctx, reference, format, valist);
  va_end(valist);
  return r;
}

#ifdef USE_STR
NODISCARD error mo_sprintf_str(struct str *const dest, char const *const reference, char const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  error err = svsprintf(dest, reference, format, valist);
  va_end(valist);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}

NODISCARD error mo_vsprintf_str(struct str *const dest,
                                char const *const reference,
                                char const *const format,
                                va_list valist) {
  return svsprintf(dest, reference, format, valist);
}
#endif
