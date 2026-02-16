#include <ovbase.h>

#include <assert.h>
#include <string.h>

#ifdef USE_MIMALLOC

#  ifdef __GNUC__
#    pragma GCC diagnostic push
#    if __has_warning("-Wreserved-identifier")
#      pragma GCC diagnostic ignored "-Wreserved-identifier"
#    endif
#  endif // __GNUC__
#  include <mimalloc.h>
#  ifdef __GNUC__
#    pragma GCC diagnostic pop
#  endif // __GNUC__

static void *ov_default_realloc(void *ptr, size_t size, void *userdata) {
  (void)userdata;
  return mi_realloc(ptr, size);
}
static void ov_default_free(void *ptr, void *userdata) {
  (void)userdata;
  mi_free(ptr);
}

#else

#  include <stdlib.h> // realloc, free

static void *ov_default_realloc(void *ptr, size_t size, void *userdata) {
  (void)userdata;
  return realloc(ptr, size);
}
static void ov_default_free(void *ptr, void *userdata) {
  (void)userdata;
  free(ptr);
}

#endif

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <ovutf.h>
#  include <windows.h>
#else
#  include <stdio.h>
#endif

static void ov_default_output(enum ov_error_severity severity, char const *const str) {
  (void)severity;
  assert(str != NULL && "str must not be NULL");
  if (!str) {
    return;
  }

#ifdef _WIN32
  HANDLE h = INVALID_HANDLE_VALUE;

  h = GetStdHandle(STD_ERROR_HANDLE);
  if (h == INVALID_HANDLE_VALUE) {
    goto cleanup;
  }

  DWORD mode;
  if (GetConsoleMode(h, &mode)) {
    wchar_t wbuf[256];
    char const *src = str;
    size_t src_len = strlen(str);

    while (src_len > 0) {
      size_t read_bytes = 0;
      size_t wlen = ov_utf8_to_wchar(src, src_len, wbuf, sizeof(wbuf) / sizeof(wbuf[0]) - 1, &read_bytes);

      if (wlen > 0 && read_bytes > 0) {
        if (!WriteConsoleW(h, wbuf, (DWORD)wlen, NULL, NULL)) {
          goto cleanup;
        }
        src += read_bytes;
        src_len -= read_bytes;
      } else {
        if (read_bytes == 0) {
          goto cleanup;
        }
        // Skip invalid byte
        src++;
        src_len--;
      }
    }
    if (!WriteConsoleW(h, L"\n", 1, NULL, NULL)) {
      goto cleanup;
    }
  } else {
    DWORD const len = (DWORD)strlen(str);
    if (!WriteFile(h, str, len, NULL, NULL)) {
      goto cleanup;
    }
    if (!WriteFile(h, "\n", 1, NULL, NULL)) {
      goto cleanup;
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
  if (fputs(str, stderr) == EOF) {
    goto cleanup;
  }
  if (fputs("\n", stderr) == EOF) {
    goto cleanup;
  }
#  ifdef __GNUC__
#    pragma GCC diagnostic pop
#  endif // __GNUC__
#endif

cleanup:
  return;
}

struct ov_init_options ov_init_get_default_options(void) {
  return (struct ov_init_options){
      .output_func = ov_default_output,
      .mem_realloc = ov_default_realloc,
      .mem_free = ov_default_free,
      .mem_userdata = NULL,
  };
}
