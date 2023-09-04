#include <ovmo.h>

#include <ovprintf.h>

#ifdef USE_STR
int mo_sprintf_char(char *const buf, size_t const buflen, char const *const reference, char const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  int const r = ov_vsnprintf(buf, buflen, reference, format, valist);
  va_end(valist);
  return r;
}

int mo_vsprintf_char(
    char *const buf, size_t const buflen, char const *const reference, char const *const format, va_list valist) {
  return ov_vsnprintf(buf, buflen, reference, format, valist);
}

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
