#include <ovbase.h>

#include <ovmo.h>
#include <ovutf.h>

#ifdef _WIN32
#  define mo_vsprintf mo_vsprintf_wstr
#else
#  define mo_vsprintf mo_vsprintf_str
#endif

NODISCARD error error_add_i18n_(error const parent,
                                int const type,
                                int const code,
                                char const *const msg ERR_FILEPOS_PARAMS) {
#ifdef _WIN32
  struct wstr ws = {0};
  error err = to_wstr(msg, strlen(msg), &ws);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return error_add_(parent, type, code, &ws ERR_FILEPOS_VALUES_PASSTHRU);
#else
  return error_add_(parent, type, code, &native_unmanaged_const(msg) ERR_FILEPOS_VALUES_PASSTHRU);
#endif
}

NODISCARD error error_add_i18nf_(error const parent,
                                 int const type,
                                 int const code ERR_FILEPOS_PARAMS,
                                 NATIVE_CHAR const *const reference,
                                 char const *const format,
                                 ...) {
  struct NATIVE_STR msg = {0};
  va_list valist;
  va_start(valist, format);
  error err = mo_vsprintf(&msg, reference, format, valist);
  va_end(valist);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return error_add_(parent, type, code, &msg ERR_FILEPOS_VALUES_PASSTHRU);
}

bool error_report_free_i18n_(error e, char const *const msg ERR_FILEPOS_PARAMS) {
#ifdef _WIN32
  struct wstr ws = {0};
  error err = to_wstr(msg, strlen(msg), &ws);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  bool const r = error_report_free_(e, &ws ERR_FILEPOS_VALUES_PASSTHRU);
  ereport(sfree(&ws));
  return r;
#else
  return error_report_free_(e, &native_unmanaged_const(msg) ERR_FILEPOS_VALUES_PASSTHRU);
#endif
}

bool error_report_free_i18nf_(error e ERR_FILEPOS_PARAMS,
                              NATIVE_CHAR const *const reference,
                              char const *const format,
                              ...) {
  struct NATIVE_STR msg = {0};
  va_list valist;
  va_start(valist, format);
  error err = mo_vsprintf(&msg, reference, format, valist);
  va_end(valist);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return error_report_free_(e, &msg ERR_FILEPOS_VALUES_PASSTHRU);
}
