#include <ovmo.h>

#include <string.h>

#include <ovarray.h>
#include <ovprintf_ex.h>
#include <ovutf.h>

bool mo_vsprintf_char(char **const dest,
                      char const *const reference,
                      char const *const format,
                      struct ov_error *const err,
                      va_list valist) {
  if (!dest || !format) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }

  return ov_vsprintf_char(dest, reference, format, err, valist);
}

bool mo_sprintf_char(
    char **const dest, char const *const reference, char const *const format, struct ov_error *const err, ...) {
  va_list valist;
  va_start(valist, err);
  bool ok = mo_vsprintf_char(dest, reference, format, err, valist);
  va_end(valist);
  if (!ok) {
    OV_ERROR_TRACE(err);
    return false;
  }
  return true;
}

bool mo_vsprintf_wchar(wchar_t **const dest,
                       wchar_t const *const reference,
                       char const *const format,
                       struct ov_error *const err,
                       va_list valist) {
  if (!dest || !format) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }

  wchar_t *wformat = NULL;
  bool result = false;

  size_t const format_len = strlen(format);
  size_t const wformat_len = ov_utf8_to_wchar_len(format, format_len);
  if (wformat_len == SIZE_MAX) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
    goto cleanup;
  }

  if (!OV_ARRAY_GROW2(&wformat, wformat_len + 1, err)) {
    OV_ERROR_TRACE(err);
    goto cleanup;
  }

  {
    size_t const converted = ov_utf8_to_wchar(format, format_len, wformat, wformat_len, NULL);
    if (converted == 0) {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
      goto cleanup;
    }
    wformat[converted] = L'\0';
    OV_ARRAY_SET_LENGTH(wformat, converted);
  }

  if (!ov_vsprintf_wchar(dest, reference, wformat, err, valist)) {
    OV_ERROR_TRACE(err);
    goto cleanup;
  }

  result = true;

cleanup:
  if (wformat) {
    OV_ARRAY_DESTROY(&wformat);
  }
  return result;
}

bool mo_sprintf_wchar(
    wchar_t **const dest, wchar_t const *const reference, char const *const format, struct ov_error *const err, ...) {
  va_list valist;
  va_start(valist, err);
  bool ok = mo_vsprintf_wchar(dest, reference, format, err, valist);
  va_end(valist);
  if (!ok) {
    OV_ERROR_TRACE(err);
    return false;
  }
  return true;
}
