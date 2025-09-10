#include <ovbase.h>

#include "mem.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <ovarray.h>
#include <ovutf.h>

#ifdef _WIN32
#  include <wchar.h>
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

static ov_error_autofill_hook_func g_autofill_hook = NULL;
static ov_error_output_hook_func g_output_hook = NULL;

void ov_error_set_autofill_hook(ov_error_autofill_hook_func hook_func) { g_autofill_hook = hook_func; }

void ov_error_set_output_hook(ov_error_output_hook_func hook_func) { g_output_hook = hook_func; }

enum {
  error_code_str_size = 16, // "[XX:0xXXXXXXXX]" + null terminator
  filepos_str_size = 256,   // enough for "file.c:12345 func_name()"
};

/**
 * Appends source string to destination buffer.
 *
 * @param dest Destination buffer (can be NULL for length calculation only)
 * @param dest_pos Current position in dest buffer
 * @param src Source string to append
 * @param src_len Length of source string
 * @return Length of the appended string
 */
static size_t append(char *dest, size_t dest_pos, char const *const src, size_t const src_len) {
  if (dest) {
    memcpy(dest + dest_pos, src, src_len);
  }
  return src_len;
}

/**
 * Converts error type and code to a formatted string representation.
 *
 * @param dest Buffer to store the result (must be at least 16 bytes), or NULL to calculate length only
 * @param type Error type (0-255)
 * @param code Error code (32-bit integer)
 * @return Length of the formatted string (excluding null terminator)
 *
 * @example
 * error_code_to_str(buf, ov_error_type_generic, ov_error_generic_out_of_memory)  -> "[01:0x00000005]"
 * error_code_to_str(buf, ov_error_type_errno,   ENOMEM)                          -> "[03:0x0000000c]"
 * error_code_to_str(buf, ov_error_type_hresult, E_OUTOFMEMORY)                   -> "[02:0x8007000e]"
 */
static size_t error_code_to_str(char *const dest, int const type, int const code) {
  if (dest) {
    static char const hex[] = "0123456789abcdef";
    unsigned const t = (unsigned)type;
    unsigned const c = (unsigned)code;
    dest[0] = '[';
    dest[1] = hex[(t >> 4) & 0xf];
    dest[2] = hex[t & 0xf];
    dest[3] = ':';
    dest[4] = '0';
    dest[5] = 'x';
    dest[6] = hex[(c >> 28) & 0xf];
    dest[7] = hex[(c >> 24) & 0xf];
    dest[8] = hex[(c >> 20) & 0xf];
    dest[9] = hex[(c >> 16) & 0xf];
    dest[10] = hex[(c >> 12) & 0xf];
    dest[11] = hex[(c >> 8) & 0xf];
    dest[12] = hex[(c >> 4) & 0xf];
    dest[13] = hex[c & 0xf];
    dest[14] = ']';
    dest[15] = '\0';
  }
  return error_code_str_size - 1;
}

/**
 * Converts file position information to a formatted string.
 *
 * @param dest Buffer to store the result (must be at least 256 bytes), or NULL to calculate length only
 * @param filepos File position structure containing file path, function name, and line number
 * @return Length of the formatted string (excluding null terminator)
 *
 * @example
 * filepos_to_str(buf, &filepos)  -> "main.c:123 my_function()"
 * filepos_to_str(buf, &filepos)  -> "utils.c:456 process_data()"
 * filepos_to_str(buf, &filepos)  -> "unkfile:0 unkfunc()"        // when file/func are NULL
 */
static size_t filepos_to_str(char *const dest, struct ov_filepos const *const filepos) {
  assert(filepos);

  static size_t const truncation_indicator_len = 3; // length of "..."

  size_t written = 0;
  bool truncated = false;

  // filename - extract just the filename
  {
    char const *file;
    if (filepos->file) {
      file = filepos->file;
      for (char const *s = filepos->file; *s != '\0'; ++s) {
        if (*s == '/' || *s == '\\') {
          file = s + 1;
        }
      }
    } else {
      file = "unkfile";
    }
    size_t const file_len = strlen(file);
    if (written + file_len >= filepos_str_size) {
      written += append(dest, written, file, filepos_str_size - written - 1);
      truncated = true;
      goto cleanup;
    }
    written += append(dest, written, file, file_len);
  }

  // :
  if (written + 1 >= filepos_str_size) {
    truncated = true;
    goto cleanup;
  }
  written += append(dest, written, ":", 1);

  // line number - convert to string
  {
    char line_str[40]; // 128bit(170141183460469231731687303715884105727) + '\0'
    static_assert(sizeof(size_t) <= 16, "size_t is too large");
    char *line = line_str + sizeof(line_str) - 1;
    *line = '\0';
    if (filepos->line == 0) {
      *--line = '0';
    } else {
      size_t n = filepos->line;
      while (n > 0) {
        *--line = '0' + (n % 10);
        n /= 10;
      }
    }
    size_t const line_len = strlen(line);
    if (written + line_len >= filepos_str_size - truncation_indicator_len) {
      truncated = true;
      goto cleanup; // do not write out incomplete line numbers as it can be confusing
    }
    written += append(dest, written, line, line_len);
  }

  // space
  if (written + 1 >= filepos_str_size) {
    truncated = true;
    goto cleanup;
  }
  written += append(dest, written, " ", 1);

  // function name
  {
    char const *const func = filepos->func ? filepos->func : "unkfunc";
    size_t const func_len = strlen(func);
    if (written + func_len >= filepos_str_size) {
      written += append(dest, written, func, filepos_str_size - written - 1);
      truncated = true;
      goto cleanup;
    }
    written += append(dest, written, func, func_len);
  }

  // ()
  if (written + 2 >= filepos_str_size) {
    truncated = true;
    goto cleanup;
  }
  written += append(dest, written, "()", 2);

cleanup:
  if (dest) {
    dest[written] = '\0';
    if (truncated && written >= truncation_indicator_len) {
      dest[written - 3] = '.';
      dest[written - 2] = '.';
      dest[written - 1] = '.';
    }
  }
  return written;
}

#define DEFINE_STATIC_STR(name, str)                                                                                   \
  static struct __attribute__((__packed__)) {                                                                          \
    struct {                                                                                                           \
      size_t len;                                                                                                      \
      size_t cap;                                                                                                      \
    } header;                                                                                                          \
    char const buf[sizeof(str)];                                                                                       \
  } const name##_str = {{.len = sizeof(str) - 1}, str};                                                                \
  static char const *const name = name##_str.buf

static char const *set_generic_message(int const code, struct ov_error *const err) {
  (void)err;

  DEFINE_STATIC_STR(success_msg, "success");
  DEFINE_STATIC_STR(fail_msg, "operation failed");
  DEFINE_STATIC_STR(abort_msg, "operation aborted");
  DEFINE_STATIC_STR(unexpected_msg, "unexpected error occurred");
  DEFINE_STATIC_STR(invalid_arg_msg, "invalid argument provided");
  DEFINE_STATIC_STR(out_of_memory_msg, "out of memory");
  DEFINE_STATIC_STR(not_implemented_msg, "feature not implemented yet");
  DEFINE_STATIC_STR(not_found_msg, "target not found");
  DEFINE_STATIC_STR(unknown_msg, "unknown error occurred");

  switch (code) {
  case ov_error_generic_success:
    return success_msg;
  case ov_error_generic_fail:
    return fail_msg;
  case ov_error_generic_abort:
    return abort_msg;
  case ov_error_generic_unexpected:
    return unexpected_msg;
  case ov_error_generic_invalid_argument:
    return invalid_arg_msg;
  case ov_error_generic_out_of_memory:
    return out_of_memory_msg;
  case ov_error_generic_not_implemented_yet:
    return not_implemented_msg;
  case ov_error_generic_not_found:
    return not_found_msg;
  case ov_error_generic_trace:
    return NULL; // trace entries don't get default messages
  default:
    return unknown_msg;
  }
}

static char const *set_errno_message(int const code, struct ov_error *const err) {
  DEFINE_STATIC_STR(unknown_errno_msg, "unknown errno.");
  char *new_msg = NULL;
  char const *result = NULL;
  {
    char const *errno_msg = strerror(code);
    if (!errno_msg) {
      result = unknown_errno_msg;
      goto cleanup;
    }

    size_t const msg_len = strlen(errno_msg);
    if (!OV_ARRAY_GROW(&new_msg, msg_len + 1, err)) {
      OV_ERROR_TRACE(err);
      goto cleanup;
    }

    strcpy(new_msg, errno_msg);
    result = new_msg;
    new_msg = NULL;
  }
cleanup:
  if (new_msg) {
    OV_ARRAY_DESTROY(&new_msg);
  }
  return result;
}

#ifdef _WIN32
static char const *set_hresult_message(int const code, struct ov_error *const err) {
  DEFINE_STATIC_STR(unavailable_msg, "Windows error message is not available.");

  LPWSTR msg = NULL;
  char *new_msg = NULL;
  char const *result = NULL;
  {
    DWORD msglen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                  NULL,
                                  (DWORD)code,
                                  MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                  (LPWSTR)&msg,
                                  0,
                                  NULL);
    if (!msglen || !msg) {
      result = unavailable_msg;
      goto cleanup;
    }

    if (msg[msglen - 1] == L'\r' || msg[msglen - 1] == L'\n') {
      msg[--msglen] = L'\0';
      if (msglen > 0 && (msg[msglen - 1] == L'\r' || msg[msglen - 1] == L'\n')) {
        msg[--msglen] = L'\0';
      }
    }

    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, msg, (int)msglen, NULL, 0, NULL, NULL);
    if (utf8_len <= 0) {
      OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
      goto cleanup;
    }

    if (!OV_ARRAY_GROW(&new_msg, (size_t)utf8_len + 1, err)) {
      OV_ERROR_TRACE(err);
      goto cleanup;
    }

    if (WideCharToMultiByte(CP_UTF8, 0, msg, (int)msglen, new_msg, utf8_len, NULL, NULL) != utf8_len) {
      OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
      goto cleanup;
    }

    new_msg[utf8_len] = '\0';
    result = new_msg;
    new_msg = NULL;
  }
cleanup:
  if (new_msg) {
    OV_ARRAY_DESTROY(&new_msg);
  }
  if (msg) {
    LocalFree(msg);
  }
  return result;
}
#endif

bool ov_error_autofill_message(struct ov_error_stack *const target, struct ov_error *const err) {
  if (!target || target->info.type == ov_error_type_invalid || target->info.context) {
    return true;
  }

  DEFINE_STATIC_STR(unknown_type_msg, "unknown error type.");
  char const *message = NULL;
  bool result = false;

  // Try custom hook first if available
  if (g_autofill_hook) {
    if (!g_autofill_hook(target->info.type, target->info.code, &message, err)) {
      // Hook failed
      OV_ERROR_TRACE(err);
      goto cleanup;
    }
    // Hook succeeded - if message is provided, use it and return
    if (message) {
      target->info.context = message;
      return true;
    }
    // Hook succeeded but no custom message - continue with default handling
  }

  // Fall back to default message generation
  switch (target->info.type) {
  case ov_error_type_generic:
    message = set_generic_message(target->info.code, err);
    if (!message) {
      // For trace entries, NULL message is expected and valid
      if (target->info.code == ov_error_generic_trace) {
        result = true;
        goto cleanup;
      }
      OV_ERROR_TRACE(err);
      goto cleanup;
    }
    break;
  case ov_error_type_errno:
    message = set_errno_message(target->info.code, err);
    if (!message) {
      OV_ERROR_TRACE(err);
      goto cleanup;
    }
    break;
#ifdef _WIN32
  case ov_error_type_hresult:
    message = set_hresult_message(target->info.code, err);
    if (!message) {
      OV_ERROR_TRACE(err);
      goto cleanup;
    }
    break;
#endif
  default:
    message = unknown_type_msg;
    break;
  }

  target->info.context = message;
  result = true;

cleanup:
  return result;
}

/**
 * Formats a single error stack entry into a human-readable string.
 *
 * @param dest Buffer to store the result, or NULL to calculate length only
 * @param dest_pos Current position in the destination buffer to start writing
 * @param entry Error stack entry containing file position, error type/code, and context message
 * @return Number of characters written (length of the formatted entry)
 *
 * @example
 * format_stack_entry(buf, 0, &entry)  -> "  main.c:123 my_function() [01:0x00000005] out of memory\n"
 * format_stack_entry(buf, 0, &entry)  -> "  utils.c:456 process_data() [03:0x0000000c] Cannot allocate memory\n"
 * format_stack_entry(buf, 0, &entry)  -> "  test.c:789 trace_func()\n"  // trace entry (no error code)
 */
static size_t format_stack_entry(char *const dest, size_t const dest_pos, struct ov_error_stack const *const entry) {
  size_t written = dest_pos;
  written += append(dest, written, "  ", 2);
  {
    char filepos_str[filepos_str_size];
    written += append(dest, written, filepos_str, filepos_to_str(dest ? filepos_str : NULL, &entry->filepos));
  }
  if (!(entry->info.type == ov_error_type_generic && entry->info.code == ov_error_generic_trace)) {
    char code_str[error_code_str_size];
    written += append(dest, written, " ", 1);
    written +=
        append(dest, written, code_str, error_code_to_str(dest ? code_str : NULL, entry->info.type, entry->info.code));
  }
  if (entry->info.context) {
    written += append(dest, written, " ", 1);
    written += append(dest, written, entry->info.context, strlen(entry->info.context));
  }
  written += append(dest, written, "\n", 1);
  return written - dest_pos;
}

/**
 * Formats a complete error message including main error and optional stack trace.
 *
 * @param dest Buffer to store the result, or NULL to calculate length only
 * @param src_copy Error structure containing the main error and stack trace entries
 * @param include_stack_trace If true, includes stack trace entries after the main error
 * @return Total length of the formatted error message
 *
 * @example
 * // Without stack trace:
 * format_error_message(buf, &error, false)  -> "[01:0x00000005] out of memory (at main.c:123 my_function())\n"
 *
 * // With stack trace:
 * format_error_message(buf, &error, true)   -> "[01:0x00000005] out of memory (at main.c:123 my_function())\n"
 *                                              "  utils.c:456 allocate_buffer() [01:0x00000005] operation failed\n"
 *                                              "  test.c:789 trace_func()\n"
 */
static size_t
format_error_message(char *const dest, struct ov_error const *const src_copy, bool const include_stack_trace) {
  size_t pos = 0;
  struct ov_error_stack const *const s0 = &src_copy->stack[0];

  // Format main message
  {
    char code_str[error_code_str_size];
    pos += append(dest, pos, code_str, error_code_to_str(dest ? code_str : NULL, s0->info.type, s0->info.code));
  }

  if (s0->info.context) {
    pos += append(dest, pos, " ", 1);
    pos += append(dest, pos, s0->info.context, strlen(s0->info.context));
  }

  // Add file position information for main error
  {
    char filepos_str[filepos_str_size];
    pos += append(dest, pos, " (at ", 5);
    pos += append(dest, pos, filepos_str, filepos_to_str(dest ? filepos_str : NULL, &s0->filepos));
    pos += append(dest, pos, ")", 1);
  }

  pos += append(dest, pos, "\n", 1);

  if (include_stack_trace) {
    for (size_t i = 1; i < sizeof(src_copy->stack) / sizeof(src_copy->stack[0]); i++) {
      if (src_copy->stack[i].info.type == ov_error_type_invalid) {
        break;
      }
      pos += format_stack_entry(dest, pos, &src_copy->stack[i]);
    }
    size_t const ext_count = OV_ARRAY_LENGTH(src_copy->stack_extended);
    for (size_t i = 0; i < ext_count; i++) {
      pos += format_stack_entry(dest, pos, &src_copy->stack_extended[i]);
    }
  }

  return pos;
}

bool ov_error_to_string(struct ov_error const *const src,
                        char **const dest,
                        bool const include_stack_trace,
                        struct ov_error *const err) {
  if (!src || !dest || src->stack[0].info.type == ov_error_type_invalid) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }

  struct ov_error src_copy = *src;
  char *initial_dest = *dest;
  bool result = false;

  // Create a local copy of the error to modify it for autofilling the message part

  for (size_t i = 0; i < sizeof(src_copy.stack) / sizeof(src_copy.stack[0]); i++) {
    if (src_copy.stack[i].info.type == ov_error_type_invalid) {
      break;
    }
    if (!ov_error_autofill_message(&src_copy.stack[i], err)) {
      OV_ERROR_TRACE(err);
      goto cleanup;
    }
  }
  {
    size_t const ext_count = OV_ARRAY_LENGTH(src->stack_extended);
    if (ext_count > 0) {
      src_copy.stack_extended = NULL;
      if (!OV_ARRAY_GROW(&src_copy.stack_extended, ext_count, err)) {
        OV_ERROR_TRACE(err);
        goto cleanup;
      }
      for (size_t i = 0; i < ext_count; i++) {
        src_copy.stack_extended[i] = src->stack_extended[i];
        if (!ov_error_autofill_message(&src_copy.stack_extended[i], err)) {
          OV_ERROR_TRACE(err);
          goto cleanup;
        }
      }
    }
  }

  {
    // Calculate total string length
    size_t const len = format_error_message(NULL, &src_copy, include_stack_trace);

    if (!OV_ARRAY_GROW(dest, len + 1, err)) {
      OV_ERROR_TRACE(err);
      goto cleanup;
    }

    // Format the error message
    size_t const pos = format_error_message(*dest, &src_copy, include_stack_trace);
    append(*dest, pos, "\0", 1);
  }

  result = true;

cleanup:
  if (!result && initial_dest == NULL && *dest) {
    OV_ARRAY_DESTROY(dest);
  }
  for (size_t i = 0; i < sizeof(src_copy.stack) / sizeof(src_copy.stack[0]); i++) {
    if (src_copy.stack[i].info.type == ov_error_type_invalid) {
      break;
    }
    if (src_copy.stack[i].info.context && src_copy.stack[i].info.context != src->stack[i].info.context) {
      OV_ARRAY_DESTROY(ov_deconster_(&src_copy.stack[i].info.context));
    }
  }
  size_t const ext_count = OV_ARRAY_LENGTH(src_copy.stack_extended);
  for (size_t i = 0; i < ext_count; i++) {
    if (src_copy.stack_extended[i].info.context &&
        src_copy.stack_extended[i].info.context != src->stack_extended[i].info.context) {
      OV_ARRAY_DESTROY(ov_deconster_(&src_copy.stack_extended[i].info.context));
    }
  }
  if (src_copy.stack_extended != src->stack_extended) {
    OV_ARRAY_DESTROY(&src_copy.stack_extended);
  }
  return result;
}

static void output(char const *const str) {
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

bool ov_error_report_and_destroy(struct ov_error *const target, char const *const message ERR_FILEPOS_PARAMS) {
  if (!target || target->stack[0].info.type == ov_error_type_invalid) {
    return true;
  }

  {
    char temp[512];
    char filepos_str[filepos_str_size];
    size_t pos = 0;
    pos += append(temp, pos, "[ERROR] ", 8);

    size_t message_len = message ? strlen(message) : 0;
    if (message_len < 200) {
      pos += append(temp, pos, message, message_len);
    } else {
      char const default_message[] = "Error occurred";
      pos += append(temp, pos, default_message, sizeof(default_message) - 1);
    }

    pos += append(temp, pos, "\n         reported at ", 22);
    pos += append(temp, pos, filepos_str, filepos_to_str(filepos_str, filepos));
    append(temp, pos, "\0", 1);

    output(temp);
  }

  {
    char fallback_msgbuf[128];
    char *error_string = NULL;
    if (!ov_error_to_string(target, &error_string, true, NULL)) {
      char codestr[error_code_str_size];
      static char const fallback_msg[] = "Cannot display error details (out of memory)";
      size_t pos = 0;
      pos += append(fallback_msgbuf, pos, fallback_msg, sizeof(fallback_msg) - 1);
      pos += append(fallback_msgbuf, pos, " ", 1);
      pos += append(fallback_msgbuf,
                    pos,
                    codestr,
                    error_code_to_str(codestr, target->stack[0].info.type, target->stack[0].info.code));
      append(fallback_msgbuf, pos, "\0", 1);
      error_string = fallback_msgbuf;
    }
    output(error_string);
    if (error_string != fallback_msgbuf) {
      OV_ARRAY_DESTROY(&error_string);
    }
  }

  ov_error_destroy(target MEM_FILEPOS_VALUES_PASSTHRU);
  return true;
}
