#include "error.h"

#include "mem.h"
#include <ovthreads.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <stdio.h> // fputs
#endif

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

NODISCARD error error_generic_message_mapper(int const type, int const code, struct NATIVE_STR *const dest) {
  if (!dest) {
    return errg(err_invalid_arugment);
  }
  if (type != err_type_generic) {
    dest->len = 0;
    return eok();
  }
  switch (code) {
  case err_fail:
    return scpy(dest, NSTR("Failed."));
  case err_unexpected:
    return scpy(dest, NSTR("Unexpected."));
  case err_invalid_arugment:
    return scpy(dest, NSTR("Invalid argument."));
  case err_null_pointer:
    return scpy(dest, NSTR("NULL pointer."));
  case err_out_of_memory:
    return scpy(dest, NSTR("Out of memory."));
  case err_not_sufficient_buffer:
    return scpy(dest, NSTR("Not sufficient buffer."));
  case err_not_found:
    return scpy(dest, NSTR("Not found."));
  case err_abort:
    return scpy(dest, NSTR("Aborted."));
  case err_not_implemented_yet:
    return scpy(dest, NSTR("Not implemented yet."));
  }
  return scpy(dest, NSTR("Unknown error code."));
}

NODISCARD error error_errno_message_mapper(int const type, int const code, struct NATIVE_STR *const dest) {
  if (!dest) {
    return errg(err_invalid_arugment);
  }
  if (type != err_type_errno) {
    dest->len = 0;
    return eok();
  }
  error err = ssprintf(dest, NULL, NSTR("errno = %d"), code);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}

#ifdef _WIN32
NODISCARD error error_win32_message_mapper(int const type,
                                           int const code,
                                           uint16_t langid,
                                           struct NATIVE_STR *const dest) {
  if (!dest) {
    return errg(err_invalid_arugment);
  }
  if (type != err_type_hresult) {
    dest->len = 0;
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
    err = scpy(dest, L"Error messages is not available.");
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    goto cleanup;
  }
  if (msg[msglen - 1] == L'\r' || msg[msglen - 1] == L'\n') {
    msg[--msglen] = L'\0';
    if (msg[msglen - 1] == L'\r' || msg[msglen - 1] == L'\n') {
      msg[--msglen] = L'\0';
    }
  }
  err = scpy(dest, msg);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  if (msg) {
    LocalFree(msg);
  }
  return err;
}
#endif

NODISCARD static error error_default_message_mapper(int const type, int const code, struct NATIVE_STR *const dest) {
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
  return scpy(dest, NSTR("Unknown error code."));
}

static error_message_mapper g_error_message_mapper = error_default_message_mapper;

void error_default_reporter(error const e,
                            struct NATIVE_STR const *const message,
                            struct ov_filepos const *const filepos) {
  struct NATIVE_STR errmsg = {0};
  struct NATIVE_STR line = {0};
  struct NATIVE_STR msg = {0};
  error err = error_to_string(e, &errmsg);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = scpy(&msg, message->ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = ssprintf(&line, NULL, NSTR("(reported at %s:%ld %s())") NEWLINE, filepos->file, filepos->line, filepos->func);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = scat(&msg, line.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = scat(&msg, errmsg.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  write_stderr(msg.ptr);

cleanup:
  if (efailed(err)) {
    write_stderr(NSTR("Failed to report error.") NEWLINE);
    efree(&err);
  }
  eignore(sfree(&msg));
  eignore(sfree(&line));
  eignore(sfree(&errmsg));
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

error error_add_(error const parent,
                 int const type,
                 int const code,
                 struct NATIVE_STR const *const msg ERR_FILEPOS_PARAMS) {
  error new_error = eok();
  error err = mem_(&new_error, 1, sizeof(struct error) MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  *new_error = (struct error){
      .type = type,
      .code = code,
      .msg = msg ? *msg : (struct NATIVE_STR){0},
      .filepos = *filepos,
  };
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
    array_free_core_((struct array *)&ee->msg MEM_FILEPOS_VALUES_PASSTHRU);
    mem_core_(&ee, 0 MEM_FILEPOS_VALUES_PASSTHRU);
    ee = next;
  }
  *e = NULL;
  return true;
}

error error_to_string_short(error e, struct NATIVE_STR *const dest) {
  if (e == NULL || (e->type == err_type_generic && e->code == err_pass_through)) {
    return errg(err_invalid_arugment);
  }
  struct NATIVE_STR tmp = {0};
  error err = g_error_message_mapper(e->type, e->code, &tmp);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  if (e->msg.len) {
    err = scatm(&tmp, NEWLINE, e->msg.ptr);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
  }
  err = scpy(dest, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  ereport(sfree(&tmp));
  return err;
}

error error_to_string(error e, struct NATIVE_STR *const dest) {
  struct NATIVE_STR msg = {0};
  struct NATIVE_STR tmp = {0};
  error err = error_to_string_short(e, &tmp);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = ssprintf(&msg,
                 NULL,
                 NEWLINE NSTR("(error code: %02X:0x%08X)") NEWLINE NSTR("  %s:%ld %s()"),
                 e->type,
                 e->code,
                 e->filepos.file,
                 e->filepos.line,
                 e->filepos.func);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = scat(&tmp, msg.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  for (e = e->next; e != NULL; e = e->next) {
    if (e->type != err_type_generic || e->code != err_pass_through) {
      err = emsg(err_type_generic, err_unexpected, &native_unmanaged(NSTR("incorrect error structure.")));
      goto cleanup;
    }
    err = ssprintf(&msg, NULL, NEWLINE NSTR("  %s:%ld %s()"), e->filepos.file, e->filepos.line, e->filepos.func);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    err = scat(&tmp, msg.ptr);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
  }
  err = scpy(dest, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  ereport(sfree(&msg));
  ereport(sfree(&tmp));
  return err;
}

bool error_report_(error const e, struct NATIVE_STR const *const message ERR_FILEPOS_PARAMS) {
  if (esucceeded(e)) {
    return true;
  }
  g_error_reporter(e, message ERR_FILEPOS_VALUES_PASSTHRU);
  return false;
}

bool error_report_free_(error e, struct NATIVE_STR const *const message ERR_FILEPOS_PARAMS) {
  bool r = error_report_(e, message ERR_FILEPOS_VALUES_PASSTHRU);
  if (!r) {
    error_free_(&e MEM_FILEPOS_VALUES_PASSTHRU);
  }
  return r;
}
