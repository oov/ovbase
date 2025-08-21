#include <ovbase.h>

#include <ovarray.h>
#include <ovmo.h>
#include <ovutf.h>

#ifdef _WIN32
#  define mo_vsprintf mo_vsprintf_wchar
#else
#  define mo_vsprintf mo_vsprintf_char
#endif

NODISCARD error error_add_i18n_(error const parent,
                                int const type,
                                int const code,
                                char const *const msg ERR_FILEPOS_PARAMS) {
  if (!msg) {
    return errg(err_invalid_arugment);
  }

  NATIVE_CHAR *native_msg = NULL;
  error err = eok();

#ifdef _WIN32
  size_t const msg_len = strlen(msg);
  size_t const native_len = ov_utf8_to_wchar_len(msg, msg_len);
  if (native_len == SIZE_MAX) {
    err = errg(err_fail);
    goto cleanup;
  }

  err = OV_ARRAY_GROW(&native_msg, native_len + 1);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  size_t converted = ov_utf8_to_wchar(msg, msg_len, native_msg, native_len, NULL);
  if (converted == 0) {
    err = errg(err_fail);
    goto cleanup;
  }
  native_msg[converted] = L'\0';
#else
  size_t const msg_len = strlen(msg);
  err = OV_ARRAY_GROW(&native_msg, msg_len + 1);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  strcpy(native_msg, msg);
#endif

  return error_add_(parent, type, code, native_msg ERR_FILEPOS_VALUES_PASSTHRU);

cleanup:
  if (native_msg) {
    OV_ARRAY_DESTROY(&native_msg);
  }
  return err;
}

NODISCARD error error_add_i18nf_(error const parent,
                                 int const type,
                                 int const code ERR_FILEPOS_PARAMS,
                                 NATIVE_CHAR const *const reference,
                                 char const *const format,
                                 ...) {
  if (!format) {
    return errg(err_invalid_arugment);
  }

  NATIVE_CHAR *msg = NULL;
  va_list valist;
  va_start(valist, format);
  error err = mo_vsprintf(&msg, reference, format, valist);
  va_end(valist);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return error_add_(parent, type, code, msg ERR_FILEPOS_VALUES_PASSTHRU);
}

bool error_report_free_i18n_(error e, char const *const msg ERR_FILEPOS_PARAMS) {
  if (!msg) {
    return error_report_free_(e, NULL ERR_FILEPOS_VALUES_PASSTHRU);
  }

  NATIVE_CHAR *native_msg = NULL;
  error err = eok();
  bool r = false;

#ifdef _WIN32
  size_t const msg_len = strlen(msg);
  size_t const native_len = ov_utf8_to_wchar_len(msg, msg_len);
  if (native_len == SIZE_MAX) {
    err = errg(err_fail);
    goto cleanup;
  }

  err = OV_ARRAY_GROW(&native_msg, native_len + 1);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

  size_t converted = ov_utf8_to_wchar(msg, msg_len, native_msg, native_len, NULL);
  if (converted == 0) {
    err = errg(err_fail);
    goto cleanup;
  }
  native_msg[converted] = L'\0';
#else
  size_t const msg_len = strlen(msg);
  err = OV_ARRAY_GROW(&native_msg, msg_len + 1);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  strcpy(native_msg, msg);
#endif

  r = error_report_free_(e, native_msg ERR_FILEPOS_VALUES_PASSTHRU);

cleanup:
  if (native_msg) {
    OV_ARRAY_DESTROY(&native_msg);
  }
  if (efailed(err)) {
    ereport(err);
    return false;
  }
  return r;
}

bool error_report_free_i18nf_(error e ERR_FILEPOS_PARAMS,
                              NATIVE_CHAR const *const reference,
                              char const *const format,
                              ...) {
  if (!format) {
    return error_report_free_(e, NULL ERR_FILEPOS_VALUES_PASSTHRU);
  }

  NATIVE_CHAR *msg = NULL;
  va_list valist;
  va_start(valist, format);
  error err = mo_vsprintf(&msg, reference, format, valist);
  va_end(valist);
  if (efailed(err)) {
    ereport(err);
    return false;
  }
  bool const r = error_report_free_(e, msg ERR_FILEPOS_VALUES_PASSTHRU);
  if (msg) {
    OV_ARRAY_DESTROY(&msg);
  }
  return r;
}
