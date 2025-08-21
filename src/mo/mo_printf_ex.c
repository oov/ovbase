#include <ovmo.h>

#include <string.h>

#include <ovarray.h>
#include <ovprintf_ex.h>
#include <ovutf.h>

NODISCARD error mo_vsprintf_char(char **const dest,
                                 char const *const reference,
                                 char const *const format,
                                 va_list valist) {
  if (!dest || !format) {
    return errg(err_invalid_arugment);
  }

  return ov_vsprintf_char(dest, reference, format, valist);
}

NODISCARD error mo_sprintf_char(char **const dest, char const *const reference, char const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  error err = mo_vsprintf_char(dest, reference, format, valist);
  va_end(valist);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}

NODISCARD error mo_vsprintf_wchar(wchar_t **const dest,
                                  wchar_t const *const reference,
                                  char const *const format,
                                  va_list valist) {
  if (!dest || !format) {
    return errg(err_invalid_arugment);
  }

  wchar_t *wformat = NULL;
  error err = eok();

  size_t const format_len = strlen(format);
  size_t const wformat_len = ov_utf8_to_wchar_len(format, format_len);
  if (wformat_len == SIZE_MAX) {
    err = errg(err_fail);
    goto cleanup;
  }

  err = OV_ARRAY_GROW(&wformat, wformat_len + 1);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  size_t converted = ov_utf8_to_wchar(format, format_len, wformat, wformat_len, NULL);
  if (converted == 0) {
    err = errg(err_fail);
    goto cleanup;
  }
  wformat[converted] = L'\0';
  OV_ARRAY_SET_LENGTH(wformat, converted);

  err = ov_vsprintf_wchar(dest, reference, wformat, valist);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  if (wformat) {
    OV_ARRAY_DESTROY(&wformat);
  }
  return err;
}

NODISCARD error mo_sprintf_wchar(wchar_t **const dest, wchar_t const *const reference, char const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  error err = mo_vsprintf_wchar(dest, reference, format, valist);
  va_end(valist);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}
