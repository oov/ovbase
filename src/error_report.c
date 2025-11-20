#include <ovbase.h>

#include "mem.h"
#include "output.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <ovarray.h>

#ifdef _WIN32
#  include <wchar.h>
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

static ov_error_autofill_hook_func g_autofill_hook = NULL;

void ov_error_set_autofill_hook(ov_error_autofill_hook_func hook_func) { g_autofill_hook = hook_func; }

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
  // dest can be NULL for length calculation only
  assert(src_len == 0 || src != NULL && "src must not be NULL when src_len > 0");
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
static size_t error_code_to_str(char dest[error_code_str_size], int const type, int const code) {
  // dest can be NULL for length calculation only
  assert(type >= 0 && "type must be non-negative");
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
static size_t filepos_to_str(char dest[filepos_str_size], struct ov_filepos const *const filepos) {
  // dest can be NULL for length calculation only
  assert(filepos != NULL && "filepos must not be NULL");

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

static bool set_generic_message(struct ov_error_stack *const target, struct ov_error *const err) {
  assert(target != NULL && "target must not be NULL");
  (void)err;
  char const *message = NULL;
  switch (target->info.code) {
  case ov_error_generic_success:
    message = "success";
    break;
  case ov_error_generic_fail:
    message = "operation failed";
    break;
  case ov_error_generic_abort:
    message = "operation aborted";
    break;
  case ov_error_generic_unexpected:
    message = "unexpected error occurred";
    break;
  case ov_error_generic_invalid_argument:
    message = "invalid argument provided";
    break;
  case ov_error_generic_out_of_memory:
    message = "out of memory";
    break;
  case ov_error_generic_not_implemented_yet:
    message = "feature not implemented yet";
    break;
  case ov_error_generic_not_found:
    message = "target not found";
    break;
  case ov_error_generic_trace:
    return true; // trace entries don't get default messages
  default:
    message = "unknown error occurred";
    break;
  }
  target->info.context = message;
  target->info.flag_context_is_static = -1;
  return true;
}

static bool set_errno_message(struct ov_error_stack *const target, struct ov_error *const err) {
  assert(target != NULL && "target must not be NULL");
  (void)err;
  target->info.context = strerror(target->info.code);
  if (!target->info.context) {
    target->info.context = "unknown errno.";
  }
  target->info.flag_context_is_static = -1;
  return true;
}

#ifdef _WIN32
static bool set_hresult_message(struct ov_error_stack *const target, struct ov_error *const err) {
  assert(target != NULL && "target must not be NULL");
  LPWSTR msg = NULL;
  char *new_msg = NULL;
  bool result = false;
  DWORD msglen = 0;
  int utf8_len = 0;

  msglen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          (DWORD)target->info.code,
                          MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                          (LPWSTR)&msg,
                          0,
                          NULL);
  if (!msglen || !msg) {
    target->info.context = "Windows error message is not available.";
    target->info.flag_context_is_static = -1;
    result = true;
    goto cleanup;
  }

  if (msg[msglen - 1] == L'\r' || msg[msglen - 1] == L'\n') {
    msg[--msglen] = L'\0';
    if (msglen > 0 && (msg[msglen - 1] == L'\r' || msg[msglen - 1] == L'\n')) {
      msg[--msglen] = L'\0';
    }
  }

  utf8_len = WideCharToMultiByte(CP_UTF8, 0, msg, (int)msglen, NULL, 0, NULL, NULL);
  if (utf8_len <= 0) {
    OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  if (!OV_ARRAY_GROW(&new_msg, (size_t)utf8_len + 1)) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
    goto cleanup;
  }

  if (WideCharToMultiByte(CP_UTF8, 0, msg, (int)msglen, new_msg, utf8_len, NULL, NULL) != utf8_len) {
    OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  new_msg[utf8_len] = '\0';
  target->info.context = new_msg;
  target->info.flag_context_is_static = 0;
  new_msg = NULL;
  result = true;

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
  assert(target != NULL && "target must not be NULL");
  // err can be NULL - error information will be ignored if NULL
  if (!target || target->info.type == ov_error_type_invalid || target->info.context) {
    return true;
  }

  bool result = false;

  // Try custom hook first if available
  if (g_autofill_hook) {
    if (!g_autofill_hook(target, err)) {
      // Hook failed
      OV_ERROR_ADD_TRACE(err);
      goto cleanup;
    }
    // Hook succeeded - if context was set by hook, return
    if (target->info.context) {
      return true;
    }
    // Hook succeeded but no custom message - continue with default handling
  }

  // Fall back to default message generation
  switch (target->info.type) {
  case ov_error_type_generic:
    if (!set_generic_message(target, err)) {
      OV_ERROR_ADD_TRACE(err);
      goto cleanup;
    }
    break;
  case ov_error_type_errno:
    if (!set_errno_message(target, err)) {
      OV_ERROR_ADD_TRACE(err);
      goto cleanup;
    }
    break;
#ifdef _WIN32
  case ov_error_type_hresult:
    if (!set_hresult_message(target, err)) {
      OV_ERROR_ADD_TRACE(err);
      goto cleanup;
    }
    break;
#endif
  default:
    target->info.context = "unknown error type.";
    target->info.flag_context_is_static = -1;
    break;
  }

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
  // dest can be NULL for length calculation only
  assert(entry != NULL && "entry must not be NULL");
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
 * @param src Error structure containing the main error and stack trace entries
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
static size_t format_error_message(char *const dest, struct ov_error const *const src, bool const include_stack_trace) {
  // dest can be NULL for length calculation only
  assert(src != NULL && "src must not be NULL");
  size_t pos = 0;
  struct ov_error_stack const *const s0 = &src->stack[0];

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
    for (size_t i = 1; i < sizeof(src->stack) / sizeof(src->stack[0]); i++) {
      if (src->stack[i].info.type == ov_error_type_invalid) {
        break;
      }
      pos += format_stack_entry(dest, pos, &src->stack[i]);
    }
    size_t const ext_count = OV_ARRAY_LENGTH(src->stack_extended);
    for (size_t i = 0; i < ext_count; i++) {
      pos += format_stack_entry(dest, pos, &src->stack_extended[i]);
    }
  }

  return pos;
}

bool ov_error_to_string(struct ov_error const *const src,
                        char **const dest,
                        bool const include_stack_trace,
                        struct ov_error *const err) {
  assert(src != NULL && "src must not be NULL");
  assert(dest != NULL && "dest must not be NULL");
  // err can be NULL - error information will be ignored if NULL
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
      OV_ERROR_ADD_TRACE(err);
      goto cleanup;
    }
  }
  {
    size_t const ext_count = OV_ARRAY_LENGTH(src->stack_extended);
    if (ext_count > 0) {
      src_copy.stack_extended = NULL;
      if (!OV_ARRAY_GROW(&src_copy.stack_extended, ext_count)) {
        OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
        goto cleanup;
      }
      OV_ARRAY_SET_LENGTH(src_copy.stack_extended, ext_count);
      for (size_t i = 0; i < ext_count; i++) {
        src_copy.stack_extended[i] = src->stack_extended[i];
        if (!ov_error_autofill_message(&src_copy.stack_extended[i], err)) {
          OV_ERROR_ADD_TRACE(err);
          goto cleanup;
        }
      }
    }
  }

  {
    // Calculate total string length
    size_t const len = format_error_message(NULL, &src_copy, include_stack_trace);

    if (!OV_ARRAY_GROW(dest, len + 1)) {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
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
      if (src_copy.stack[i].info.flag_context_is_static) {
        src_copy.stack[i].info.context = NULL;
      } else {
        OV_ARRAY_DESTROY(ov_deconster_(&src_copy.stack[i].info.context));
      }
    }
  }
  size_t const ext_count = OV_ARRAY_LENGTH(src_copy.stack_extended);
  for (size_t i = 0; i < ext_count; i++) {
    if (src_copy.stack_extended[i].info.context &&
        src_copy.stack_extended[i].info.context != src->stack_extended[i].info.context) {
      if (src_copy.stack_extended[i].info.flag_context_is_static) {
        src_copy.stack_extended[i].info.context = NULL;
      } else {
        OV_ARRAY_DESTROY(ov_deconster_(&src_copy.stack_extended[i].info.context));
      }
    }
  }
  if (src_copy.stack_extended != src->stack_extended) {
    OV_ARRAY_DESTROY(&src_copy.stack_extended);
  }
  return result;
}

void ov_error_report_and_destroy(struct ov_error *const target,
                                 enum ov_error_severity severity,
                                 char const *const message ERR_FILEPOS_PARAMS) {
  assert(target != NULL && "target must not be NULL");
  // message can be NULL - a default message will be used
  assert(filepos != NULL && "filepos must not be NULL");
  if (!target) {
    return; // accept NULL target, do nothing
  }
  if (!filepos) {
    return; // invalid parameters
  }
  if (target->stack[0].info.type == ov_error_type_invalid) {
    return; // no error to report
  }
  {
    char temp[512];
    char filepos_str[filepos_str_size];
    size_t pos = 0;
    switch (severity) {
    case ov_error_severity_error:
      pos += append(temp, pos, "[ERROR] ", 8);
      break;
    case ov_error_severity_warn:
      pos += append(temp, pos, "[WARN] ", 7);
      break;
    case ov_error_severity_info:
      pos += append(temp, pos, "[INFO] ", 7);
      break;
    case ov_error_severity_verbose:
      pos += append(temp, pos, "[VERBOSE] ", 10);
      break;
    }

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

    output(severity, temp);
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
    output(severity, error_string);
    if (error_string != fallback_msgbuf) {
      OV_ARRAY_DESTROY(&error_string);
    }
  }

  ov_error_destroy(target MEM_FILEPOS_VALUES_PASSTHRU);
}
