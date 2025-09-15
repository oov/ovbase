#include "output.h"

#include <assert.h>
#include <ovutf.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <stdio.h>
#endif

static ov_error_output_hook_func g_output_hook = NULL;
void ov_error_set_output_hook(ov_error_output_hook_func hook_func) {
  // hook_func can be NULL to restore default stderr output
  g_output_hook = hook_func;
}

void output(char const *const str) {
  assert(str != NULL && "str must not be NULL");
  if (!str) {
    return;
  }

  // Use custom output hook if available
  if (g_output_hook) {
    g_output_hook(str);
    return;
  }

  // Default implementation: write to stderr with UTF-8 encoding
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
