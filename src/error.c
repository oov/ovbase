#include "error.h"

#include "../3rd/hashmap.c/hashmap.h"
#include "mem.h"
#include "ovthreads.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <stdio.h> // fprintf
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
  fprintf(stderr, NSTR("%s") NEWLINE, str);
#endif
}

void error_default_reporter(error const e,
                            struct NATIVE_STR const *const message,
                            struct ovbase_filepos const *const filepos) {
  struct NATIVE_STR tmp = {0};
  struct NATIVE_STR msg = {0};
  error err = error_to_string(e, &tmp);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = scpy(&msg, message->ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  NATIVE_CHAR buf[1024] = {0};
#ifdef _WIN32
  wsprintfW(buf, NEWLINE NSTR("(reported at %hs:%ld %hs())") NEWLINE, filepos->file, filepos->line, filepos->func);
#else
  sprintf(buf, NEWLINE NSTR("(reported at %s:%ld %s())") NEWLINE, filepos->file, filepos->line, filepos->func);
#endif
  err = scat(&msg, buf);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = scat(&msg, tmp.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  write_stderr(msg.ptr);

cleanup:
  if (efailed(err)) {
    write_stderr(NSTR("failed to report error"));
    efree(&err);
  }
  eignore(sfree(&msg));
  eignore(sfree(&tmp));
}

static mtx_t g_error_mtx = {0};
static struct hashmap *g_error_message_mapper = NULL;
static error_message_reporter g_error_reporter = error_default_reporter;

struct error_message_mapping {
  size_t type;
  NODISCARD error_message_mapper get;
};

static uint64_t emm_hash(void const *const item, uint64_t const seed0, uint64_t const seed1, void *const udata) {
  (void)udata;
  return hashmap_sip(item, sizeof(size_t), seed0, seed1);
}

static int emm_compare(void const *a, void const *b, void *const udata) {
  (void)udata;
  return (int)(*(size_t const *)a - *(size_t const *)b);
}

#ifdef _WIN32
NODISCARD static error win32_error_message_mapper(uint_least32_t const code, struct NATIVE_STR *const dest) {
  if (!dest) {
    return errg(err_invalid_arugment);
  }

  error err = eok();
  LPWSTR msg = NULL;
  DWORD msglen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                (DWORD)code,
                                LANG_USER_DEFAULT,
                                (LPWSTR)&msg,
                                0,
                                NULL);
  if (!msglen) {
    ereport(errhr(HRESULT_FROM_WIN32(GetLastError())));
    err = scpy(dest, L"error messages is not available.");
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

  goto cleanup;

cleanup:
  if (msg) {
    LocalFree(msg);
  }
  return err;
}
#endif

bool error_init(void) {
  mtx_init(&g_error_mtx, mtx_plain);
  uint64_t hash = ovbase_splitmix64_next(get_global_hint());
  uint64_t const s0 = ovbase_splitmix64(hash);
  hash = ovbase_splitmix64_next(hash);
  uint64_t const s1 = ovbase_splitmix64(hash);
  g_error_message_mapper = hashmap_new_with_allocator(ovbase_hm_malloc,
                                                      ovbase_hm_free,
                                                      sizeof(struct error_message_mapping),
                                                      1,
                                                      s0,
                                                      s1,
                                                      emm_hash,
                                                      emm_compare,
                                                      NULL,
                                                      NULL);
  if (!g_error_message_mapper) {
    goto failed;
  }
  return true;

failed:
  if (g_error_message_mapper) {
    hashmap_free(g_error_message_mapper);
    g_error_message_mapper = NULL;
  }
  mtx_destroy(&g_error_mtx);
  return false;
}

bool error_register_default_mapper(error_message_mapper generic_error_message_mapper) {
  error err = error_register_message_mapper(err_type_generic, generic_error_message_mapper);
  if (efailed(err)) {
    goto failed;
  }
#ifdef _WIN32
  err = error_register_message_mapper(err_type_hresult, win32_error_message_mapper);
  if (efailed(err)) {
    goto failed;
  }
#endif
  return true;

failed:
  efree(&err);
  return false;
}

void error_exit(void) {
  if (g_error_message_mapper) {
    hashmap_free(g_error_message_mapper);
    g_error_message_mapper = NULL;
    mtx_destroy(&g_error_mtx);
  }
}

static error find_last_error(error e) {
  if (!e) {
    return NULL;
  }
  while (e->next != NULL) {
    e = e->next;
  }
  return e;
}

error error_register_message_mapper(int const type, error_message_mapper fn) {
  mtx_lock(&g_error_mtx);
  void *r = hashmap_set(g_error_message_mapper,
                        &(struct error_message_mapping){
                            .type = (size_t const)type,
                            .get = fn,
                        });
  error err = r == NULL && hashmap_oom(g_error_message_mapper) ? errg(err_out_of_memory) : eok();
  mtx_unlock(&g_error_mtx);
  return err;
}

void error_register_reporter(error_message_reporter const fn) {
  mtx_lock(&g_error_mtx);
  g_error_reporter = fn ? fn : error_default_reporter;
  mtx_unlock(&g_error_mtx);
}

NODISCARD static error error_get_registered_message_mapper(int const type, struct error_message_mapping **const em) {
  mtx_lock(&g_error_mtx);
  struct error_message_mapping *found =
      hashmap_get(g_error_message_mapper, &(struct error_message_mapping){.type = (size_t const)type});
  error err = found ? eok() : errg(err_not_found);
  if (esucceeded(err)) {
    *em = found;
  }
  mtx_unlock(&g_error_mtx);
  return err;
}

error error_add_(error const parent,
                 int const type,
                 uint_least32_t const code,
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
  struct error_message_mapping *em = NULL;
  error err = error_get_registered_message_mapper(e->type, &em);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = em->get(e->code, &tmp);
  if (efailed(err)) {
    ereport(err);
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
  NATIVE_CHAR buf[1024] = {0};
  struct NATIVE_STR tmp = {0};

  error err = error_to_string_short(e, &tmp);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
#ifdef _WIN32
  wsprintfW(buf,
            NEWLINE NSTR("(error code: %02X:0x%08X)") NEWLINE NSTR("  %hs:%ld %hs()"),
            e->type,
            e->code,
            e->filepos.file,
            e->filepos.line,
            e->filepos.func);
#else
  sprintf(buf,
          NEWLINE NSTR("(error code: %02X:0x%08X)") NEWLINE NSTR("  %s:%ld %s()"),
          e->type,
          e->code,
          e->filepos.file,
          e->filepos.line,
          e->filepos.func);
#endif
  err = scat(&tmp, buf);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  for (e = e->next; e != NULL; e = e->next) {
    if (e->type != err_type_generic || e->code != err_pass_through) {
      err = emsg(err_type_generic, err_unexpected, &native_unmanaged(NSTR("incorrect error structure.")));
      goto cleanup;
    }
#ifdef _WIN32
    wsprintfW(buf, NEWLINE NSTR("  %hs:%ld %hs()"), e->filepos.file, e->filepos.line, e->filepos.func);
#else
    sprintf(buf, NEWLINE NSTR("  %s:%ld %s()"), e->filepos.file, e->filepos.line, e->filepos.func);
#endif
    err = scat(&tmp, buf);
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

bool error_report_(error const e, struct NATIVE_STR const *const message ERR_FILEPOS_PARAMS) {
  if (esucceeded(e)) {
    return true;
  }

  mtx_lock(&g_error_mtx);
  error_message_reporter fn = g_error_reporter;
  mtx_unlock(&g_error_mtx);
  fn(e, message ERR_FILEPOS_VALUES_PASSTHRU);
  return false;
}

bool error_report_free_(error e, struct NATIVE_STR const *const message ERR_FILEPOS_PARAMS) {
  bool r = error_report_(e, message ERR_FILEPOS_VALUES_PASSTHRU);
  if (!r) {
    error_free_(&e MEM_FILEPOS_VALUES_PASSTHRU);
  }
  return r;
}
