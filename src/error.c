#include "error.h"

#include "mem.h"
#include <ovarray.h>
#include <ovprintf.h>
#include <ovthreads.h>

#ifdef _WIN32
#  include <wchar.h>
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <stdio.h> // fputs
#  include <string.h>
#endif

#ifdef _WIN32
#  define STRLEN wcslen
#  define STRCPY wcscpy
#else
#  define STRLEN strlen
#  define STRCPY strcpy
#endif // _WIN32

static void write_stderr(NATIVE_CHAR const *const str) {
#ifdef _WIN32
  // https://docs.microsoft.com/en-us/windows/console/writeconsole
  // WriteConsole fails if it is used with a standard handle that is redirected
  // to a file. If an application processes multilingual output that can be
  // redirected, determine whether the output handle is a console handle (one
  // method is to call the GetConsoleMode function and check whether it
  // succeeds). If the handle is a console handle, call WriteConsole. If the
  // handle is not a console handle, the output is redirected and you should
  // call WriteFile to perform the I/O. Be sure to prefix a Unicode plain text
  // file with a byte order mark. For more information, see Using Byte Order
  // Marks.
  HANDLE const h = GetStdHandle(STD_ERROR_HANDLE);
  DWORD const len = (DWORD)wcslen(str);
  if (GetConsoleMode(h, &(DWORD){0})) {
    WriteConsoleW(h, str, len, NULL, NULL);
  } else {
    DWORD const plen = (DWORD)WideCharToMultiByte(CP_UTF8, 0, str, (int)len, NULL, 0, NULL, NULL);
    if (plen) {
      void *const p = malloc(plen);
      if (WideCharToMultiByte(CP_UTF8, 0, str, (int)len, p, (int)plen, NULL, NULL)) {
        WriteFile(h, p, plen, NULL, NULL);
      }
      free(p);
    }
  }
#else
#  ifdef __GNUC__
#    ifndef __has_warning
#      define __has_warning(x) 0
#    endif
#    pragma GCC diagnostic push
#    if __has_warning("-Wdisabled-macro-expansion")
#      pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#    endif
#  endif
  fputs(str, stderr);
  fputs(NEWLINE, stderr);
#  ifdef __GNUC__
#    pragma GCC diagnostic pop
#  endif // __GNUC__
#endif
}

static NATIVE_CHAR const *get_generic_error_message(int const code) {
  switch (code) {
  case err_fail:
    return NSTR("Failed.");
  case err_unexpected:
    return NSTR("Unexpected.");
  case err_invalid_arugment:
    return NSTR("Invalid argument.");
  case err_null_pointer:
    return NSTR("NULL pointer.");
  case err_out_of_memory:
    return NSTR("Out of memory.");
  case err_not_sufficient_buffer:
    return NSTR("Not sufficient buffer.");
  case err_not_found:
    return NSTR("Not found.");
  case err_abort:
    return NSTR("Aborted.");
  case err_not_implemented_yet:
    return NSTR("Not implemented yet.");
  default:
    return NSTR("Unknown error code.");
  }
}

NODISCARD error error_generic_message_mapper(int const type, int const code, NATIVE_CHAR **const dest) {
  if (!dest) {
    return errg(err_invalid_arugment);
  }
  if (type != err_type_generic) {
    return eok();
  }
  NATIVE_CHAR const *msg = get_generic_error_message(code);
  error err = OV_ARRAY_GROW(dest, STRLEN(msg));
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  STRCPY(*dest, msg);
  return err;
}

NODISCARD error error_errno_message_mapper(int const type, int const code, NATIVE_CHAR **const dest) {
  if (!dest) {
    return errg(err_invalid_arugment);
  }
  if (type != err_type_errno) {
    return eok();
  }
  NATIVE_CHAR buf[256];
  ov_snprintf(buf, sizeof(buf), NULL, NSTR("errno = %d"), code);
  error err = OV_ARRAY_GROW(dest, STRLEN(buf));
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  STRCPY(*dest, buf);
  return eok();
}

#ifdef _WIN32
NODISCARD error error_win32_message_mapper(int const type, int const code, uint16_t langid, NATIVE_CHAR **const dest) {
  if (!dest) {
    return errg(err_invalid_arugment);
  }
  if (type != err_type_hresult) {
    return eok();
  }
  error err = eok();
  LPWSTR msg = NULL;
  DWORD msglen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                (DWORD)code,
                                (DWORD)langid,
                                (LPWSTR)&msg,
                                0,
                                NULL);
  if (!msglen) {
    ereport(errhr(HRESULT_FROM_WIN32(GetLastError())));
    static NATIVE_CHAR const error_msg[] = NSTR("Error messages is not available.");
    err = OV_ARRAY_GROW(dest, STRLEN(error_msg) + 1);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    STRCPY(*dest, error_msg);
    goto cleanup;
  }
  if (msg[msglen - 1] == L'\r' || msg[msglen - 1] == L'\n') {
    msg[--msglen] = L'\0';
    if (msg[msglen - 1] == L'\r' || msg[msglen - 1] == L'\n') {
      msg[--msglen] = L'\0';
    }
  }
  err = OV_ARRAY_GROW(dest, STRLEN(msg) + 1);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  STRCPY(*dest, msg);
cleanup:
  if (msg) {
    LocalFree(msg);
  }
  return err;
}
#endif

NODISCARD static error error_default_message_mapper(int const type, int const code, NATIVE_CHAR **const dest) {
  if (type == err_type_generic) {
    return error_generic_message_mapper(type, code, dest);
  }
#ifdef _WIN32
  if (type == err_type_hresult) {
    return error_win32_message_mapper(type, code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), dest);
  }
#endif
  if (type == err_type_errno) {
    return error_errno_message_mapper(type, code, dest);
  }
  static NATIVE_CHAR const unknown_msg[] = NSTR("Unknown error code.");
  error err = OV_ARRAY_GROW(dest, STRLEN(unknown_msg) + 1);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  STRCPY(*dest, unknown_msg);
  return err;
}

static error_message_mapper g_error_message_mapper = error_default_message_mapper;

void error_default_reporter(error const e, NATIVE_CHAR const *const message, struct ov_filepos const *const filepos) {
  NATIVE_CHAR *errmsg = NULL;
  NATIVE_CHAR buf[1024];
  error err = error_to_string(e, &errmsg);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  ov_snprintf(&buf[0],
              sizeof(buf),
              NULL,
              (NSTR_PH NSTR("(reported at %hs:%ld %hs())") NEWLINE NSTR_PH),
              message,
              filepos->file,
              filepos->line,
              filepos->func,
              errmsg);
  write_stderr(buf);
cleanup:
  if (efailed(err)) {
    write_stderr(NSTR("Failed to report error.") NEWLINE);
    efree(&err);
  }
  if (errmsg) {
    OV_ARRAY_DESTROY(&errmsg);
  }
}

static error_message_reporter g_error_reporter = error_default_reporter;

void error_set_message_mapper(error_message_mapper fn) { g_error_message_mapper = fn; }
void error_set_reporter(error_message_reporter fn) { g_error_reporter = fn; }

static error find_last_error(error e) {
  if (!e) {
    return NULL;
  }
  while (e->next != NULL) {
    e = e->next;
  }
  return e;
}

error error_add_(error const parent, int const type, int const code, NATIVE_CHAR const *const msg ERR_FILEPOS_PARAMS) {
  error new_error = eok();
  error err = mem_(&new_error, 1, sizeof(struct error) MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  *new_error = (struct error){
      .type = type,
      .code = code,
      .filepos = *filepos,
  };
  if (msg) {
    if (!ov_array_grow((void **)&new_error->msg, sizeof(NATIVE_CHAR), STRLEN(msg) + 1 MEM_FILEPOS_VALUES_PASSTHRU)) {
      return errg(err_out_of_memory);
    }
    STRCPY(new_error->msg, msg);
  }
  error last_error = find_last_error(parent);
  if (last_error) {
    last_error->next = new_error;
    return parent;
  }
  return new_error;
}
bool error_free_(error *const e MEM_FILEPOS_PARAMS) {
  if (!e) {
    return false;
  }
  error ee = *e;
  while (ee != NULL) {
    error next = ee->next;
    if (ee->msg) {
      ov_array_destroy((void **)&ee->msg MEM_FILEPOS_VALUES_PASSTHRU);
    }
    mem_core_(&ee, 0 MEM_FILEPOS_VALUES_PASSTHRU);
    ee = next;
  }
  *e = NULL;
  return true;
}
error error_to_string_short(error e, NATIVE_CHAR **const dest) {
  if (e == NULL || (e->type == err_type_generic && e->code == err_pass_through)) {
    return errg(err_invalid_arugment);
  }
  NATIVE_CHAR *tmp = NULL;
  error err = g_error_message_mapper(e->type, e->code, &tmp);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  if (e->msg) {
    size_t const tmplen = STRLEN(tmp);
    size_t const nrlen = STRLEN(NEWLINE);
    size_t const msglen = STRLEN(e->msg);
    err = OV_ARRAY_GROW(dest, tmplen + nrlen + msglen + 1);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    STRCPY(*dest, tmp);
    STRCPY(*dest + tmplen, NEWLINE);
    STRCPY(*dest + tmplen + nrlen, e->msg);
  } else {
    err = OV_ARRAY_GROW(dest, STRLEN(tmp) + 1);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    STRCPY(*dest, tmp);
  }
cleanup:
  if (tmp) {
    OV_ARRAY_DESTROY(&tmp);
  }
  return err;
}

error error_to_string(error e, NATIVE_CHAR **const dest) {
  NATIVE_CHAR buf[1024];
  NATIVE_CHAR *tmp = NULL;
  error err = error_to_string_short(e, &tmp);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  ov_snprintf(buf,
              sizeof(buf),
              NULL,
              (NEWLINE NSTR("(error code: %02X:0x%08X)") NEWLINE NSTR("  %hs:%ld %hs()")),
              e->type,
              e->code,
              e->filepos.file,
              e->filepos.line,
              e->filepos.func);

  size_t const tmplen = STRLEN(tmp);
  size_t const buflen = STRLEN(buf);
  err = OV_ARRAY_GROW(dest, tmplen + buflen + 1);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  STRCPY(*dest, tmp);
  STRCPY(*dest + tmplen, buf);

  for (e = e->next; e != NULL; e = e->next) {
    if (e->type != err_type_generic || e->code != err_pass_through) {
      static NATIVE_CHAR const error_msg[] = NSTR("incorrect error structure.");
      err = OV_ARRAY_GROW(dest, STRLEN(error_msg) + 1);
      if (efailed(err)) {
        err = ethru(err);
        goto cleanup;
      }
      STRCPY(*dest, error_msg);
      err = errg(err_unexpected);
      goto cleanup;
    }
    ov_snprintf(
        buf, sizeof(buf), NULL, NEWLINE NSTR("  %hs:%ld %hs()"), e->filepos.file, e->filepos.line, e->filepos.func);
    size_t const destlen = STRLEN(*dest);
    size_t const newbuflen = STRLEN(buf);
    err = OV_ARRAY_GROW(dest, destlen + newbuflen + 1);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    STRCPY(*dest + destlen, buf);
  }

cleanup:
  if (tmp) {
    OV_ARRAY_DESTROY(&tmp);
  }
  return err;
}

bool error_report_(error const e, NATIVE_CHAR const *const message ERR_FILEPOS_PARAMS) {
  if (esucceeded(e)) {
    return true;
  }
  g_error_reporter(e, message ERR_FILEPOS_VALUES_PASSTHRU);
  return false;
}

bool error_report_free_(error e, NATIVE_CHAR const *const message ERR_FILEPOS_PARAMS) {
  bool r = error_report_(e, message ERR_FILEPOS_VALUES_PASSTHRU);
  if (!r) {
    error_free_(&e MEM_FILEPOS_VALUES_PASSTHRU);
  }
  return r;
}
