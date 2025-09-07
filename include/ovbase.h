#pragma once

#include <ovbase_config.h>

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __has_c_attribute
#  define __has_c_attribute(x) 0
#endif
#ifndef __has_attribute
#  define __has_attribute(x) 0
#endif
#ifndef __has_warning
#  define __has_warning(x) 0
#endif

#if __has_attribute(warn_unused_result)
#  define NODISCARD __attribute__((warn_unused_result))
#elif __has_c_attribute(nodiscard)
#  define NODISCARD [[nodiscard]]
#else
#  define NODISCARD
#endif

#if __has_attribute(noreturn)
#  define NORETURN __attribute__((noreturn))
#elif __has_c_attribute(noreturn)
#  define NORETURN [[noreturn]]
#else
#  define NORETURN
#endif

#ifdef __FILE_NAME__
#  define SOURCE_CODE_FILE_NAME __FILE_NAME__
#else
#  define SOURCE_CODE_FILE_NAME __FILE__
#endif

struct ov_filepos {
  char const *file;
  char const *func;
  size_t line;
};

#define ERR_FILEPOS_PARAMS , struct ov_filepos const *const filepos
#define ERR_FILEPOS_VALUES                                                                                             \
  , (&(const struct ov_filepos){.file = SOURCE_CODE_FILE_NAME, .func = __func__, .line = __LINE__})
#define ERR_FILEPOS_VALUES_PASSTHRU , filepos

#ifdef ALLOCATE_LOGGER
#  define MEM_FILEPOS_PARAMS ERR_FILEPOS_PARAMS
#  define MEM_FILEPOS_VALUES ERR_FILEPOS_VALUES
#  define MEM_FILEPOS_VALUES_PASSTHRU ERR_FILEPOS_VALUES_PASSTHRU
#else
#  define MEM_FILEPOS_PARAMS
#  define MEM_FILEPOS_VALUES
#  define MEM_FILEPOS_VALUES_PASSTHRU
#endif

#ifdef _WIN32
#  define NATIVE_CHAR wchar_t
#  define NSTR(str) L##str
#  define NSTR_PH NSTR("%ls")
#  define NEWLINE NSTR("\r\n")
#else
#  define NATIVE_CHAR char
#  define NSTR(str) str
#  define NSTR_PH NSTR("%s")
#  define NEWLINE NSTR("\n")
#endif

static inline void *ov_deconster_(void const *const ptr) {
#ifdef __GNUC__
#  pragma GCC diagnostic push
#  if __has_warning("-Wcast-qual")
#    pragma GCC diagnostic ignored "-Wcast-qual"
#  endif
  return (void *)ptr;
#  pragma GCC diagnostic pop
#else
  return (void *)ptr;
#endif // __GNUC__
}

// error

enum ov_error_type {
  ov_error_type_trace = -1,
  ov_error_type_invalid = 0,
  ov_error_type_generic = 1,
  ov_error_type_hresult = 2,
  ov_error_type_errno = 3,
};

enum ov_error_generic {
  ov_error_generic_success = 0,
  ov_error_generic_fail = 1,
  ov_error_generic_abort = 2,
  ov_error_generic_unexpected = 3,
  ov_error_generic_invalid_argument = 4,
  ov_error_generic_out_of_memory = 5,
  ov_error_generic_not_implemented_yet = 6,
  ov_error_generic_not_found = 7,
};

/**
 * @brief Internal error information structure - not for direct user access
 *
 * This structure is used internally by the error system to store error type,
 * code, and optional context message. Users should not directly create or
 * manipulate instances of this structure. Instead, use the provided macros
 * like OV_ERROR_SET, OV_ERROR_DEFINE, OV_ERROR_SETF, etc.
 *
 * @internal This is an internal implementation detail. User code should not
 * directly access or modify fields of this structure.
 *
 * @see OV_ERROR_SET, OV_ERROR_DEFINE, OV_ERROR_SETF for user-facing interfaces
 */
struct ov_error_info {
  int type;
  int code;
  char const *context;
};

/**
 * @brief Internal error stack entry - not for direct user access
 *
 * This structure represents a single entry in the error stack, combining
 * error information with file position data for debugging and stack trace
 * functionality. Users should not directly create or manipulate instances
 * of this structure. The error system manages these entries automatically
 * when using error handling macros.
 *
 * @internal This is an internal implementation detail. User code should not
 * directly access or modify fields of this structure. The error stack is
 * managed automatically by the error system.
 *
 * @see struct ov_error for the user-facing error container
 * @see OV_ERROR_TRACE, OV_ERROR_PUSH for stack manipulation
 */
struct ov_error_stack {
  struct ov_error_info info;
  struct ov_filepos filepos;
};

/**
 * @brief Modern error handling structure for ovbase library
 *
 * This structure provides comprehensive error handling with stack trace support.
 * It should be used as the primary error handling mechanism in new code.
 * The structure automatically manages memory for error messages and provides
 * detailed debugging information including file, function, and line information.
 *
 * ## Basic Usage Pattern
 *
 * Functions should follow this pattern for consistent error handling:
 *
 * ```c
 * bool example_func(void **param, struct ov_error *const err) {
 *   // Simple argument validation first
 *   if (!param || !*param) {
 *     OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
 *     return false;
 *   }
 *
 *   // Initialize resources at the beginning
 *   void *ptr = NULL;
 *   wchar_t *str = NULL;
 *   HANDLE h = INVALID_HANDLE_VALUE;
 *   bool result = false;
 *
 *   // Use goto cleanup pattern instead of early returns
 *   if (!some_func(&ptr, err)) {
 *     OV_ERROR_TRACE(err); // Add stack trace if not generating new error
 *     goto cleanup;
 *   }
 *
 *   // Handle Win32 API errors
 *   h = CreateFileW(...);
 *   if (h == INVALID_HANDLE_VALUE) {
 *     HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
 *     if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
 *       goto cleanup; // Treat as success
 *     }
 *     OV_ERROR_SET_HRESULT(err, hr);
 *     goto cleanup;
 *   }
 *
 *   // Handle specific error cases when needed
 *   bool ok = another_func(&str);
 *   if (!ok) {
 *     if (ov_error_is(err, ov_error_type_generic, ov_error_generic_invalid_argument)) {
 *       OV_ERROR_DESTROY(err); // Clean up error for success case
 *       goto cleanup; // Treat as success
 *     }
 *     OV_ERROR_TRACE(err);
 *     goto cleanup;
 *   }
 *
 *   // Set success flag just before cleanup
 *   result = true;
 *
 * cleanup:
 *   if (h != INVALID_HANDLE_VALUE) {
 *     CloseHandle(h);
 *     h = INVALID_HANDLE_VALUE;
 *   }
 *   if (ptr) {
 *     OV_FREE(&ptr);
 *   }
 *   if (str) {
 *     OV_ARRAY_DESTROY(&str);
 *   }
 *   return result;
 * }
 * ```
 *
 * ## Key Guidelines
 * - Always initialize to {0}: `struct ov_error err = {0};`
 * - Initialize result variable to false at the beginning
 * - Use goto cleanup pattern to avoid resource leaks
 * - Add OV_ERROR_TRACE() when passing errors up without modification
 * - Use OV_ERROR_DESTROY() to clean up error memory when needed
 * - Never use early returns before cleanup
 * - Set result = true only when all operations succeed, just before cleanup
 * - NEVER check error contents directly (err->type == 0, etc.) as err might be NULL
 *
 * @see OV_ERROR_SET, OV_ERROR_SET_GENERIC, OV_ERROR_SETF for setting errors
 * @see OV_ERROR_TRACE, OV_ERROR_PUSH for adding stack context
 * @see OV_ERROR_DESTROY for cleanup
 * @see ov_error_is, ov_error_get_code for error inspection
 */
struct ov_error {
  struct ov_error_stack stack[8];
  struct ov_error_stack *stack_extended;
};

/**
 * @brief Low-level implementation for OV_ERROR_DESTROY
 *
 * Use OV_ERROR_DESTROY macro instead of calling this function directly.
 *
 * @param target Pointer to struct ov_error to destroy
 *
 * @see OV_ERROR_DESTROY
 */
void ov_error_destroy(struct ov_error *const target MEM_FILEPOS_PARAMS);

/**
 * @brief Low-level implementation for OV_ERROR_SET
 *
 * Use OV_ERROR_SET macro instead of calling this function directly.
 *
 * @param target Pointer to struct ov_error
 * @param info Pointer to error information
 *
 * @see OV_ERROR_SET
 */
void ov_error_set(struct ov_error *const target, struct ov_error_info const *const info ERR_FILEPOS_PARAMS);

/**
 * @brief Low-level implementation for OV_ERROR_SETF
 *
 * Use OV_ERROR_SETF macro instead of calling this function directly.
 *
 * @param target Pointer to struct ov_error
 * @param info Pointer to error information
 * @param reference Reference format string
 * @param ... Format arguments
 *
 * @see OV_ERROR_SETF
 */
void ov_error_setf(struct ov_error *const target,
                   struct ov_error_info const *const info,
                   char const *const reference ERR_FILEPOS_PARAMS,
                   ...);

/**
 * @brief Low-level implementation for OV_ERROR_PUSH
 *
 * Use OV_ERROR_PUSH macro instead of calling this function directly.
 *
 * @param target Pointer to struct ov_error with existing error
 * @param info Pointer to error information to add
 *
 * @see OV_ERROR_PUSH
 */
void ov_error_push(struct ov_error *const target, struct ov_error_info const *const info ERR_FILEPOS_PARAMS);

/**
 * @brief Low-level implementation for OV_ERROR_PUSHF
 *
 * Use OV_ERROR_PUSHF macro instead of calling this function directly.
 *
 * @param target Pointer to struct ov_error with existing error
 * @param info Pointer to error information
 * @param reference Reference format string
 * @param ... Format arguments
 *
 * @see OV_ERROR_PUSHF
 */
void ov_error_pushf(struct ov_error *const target,
                    struct ov_error_info const *const info,
                    char const *const reference ERR_FILEPOS_PARAMS,
                    ...);

/**
 * Hook function type for custom error message generation
 *
 * This callback allows customization of error messages for any error type/code combination.
 * The hook is called before the standard message generation to provide custom handling.
 *
 * @param type Error type (e.g., ov_error_type_generic, ov_error_type_hresult, ov_error_type_errno)
 * @param code Error code (e.g., ov_error_generic_fail, HRESULT, errno value)
 * @param message_out Pointer to store the custom message string. Set to NULL if no custom message available.
 * @param err Pointer to struct ov_error for error information if message generation fails
 * @return true on success (including when no custom message available), false on failure
 *
 * @example
 * static bool my_hook(int type, int code, char const **message_out, struct ov_error *err) {
 *   if (type == ov_error_type_generic && code == ov_error_generic_fail) {
 *     OV_ERROR_DEFINE(custom_msg, ov_error_type_generic, ov_error_generic_fail, "Custom failure message");
 *     *message_out = custom_msg.context;
 *     return true;
 *   }
 *   *message_out = NULL; // No custom message, use default
 *   return true;
 * }
 */
typedef bool (*ov_error_autofill_hook_func)(int type, int code, char const **message_out, struct ov_error *err);

/**
 * Set custom hook function for error message autofilling
 *
 * Registers a custom hook function that will be called before standard error message
 * generation. The hook can provide custom messages for any error type/code combination,
 * allowing complete override or extension of the default error messaging system.
 *
 * @param hook_func Hook function pointer, or NULL to disable custom hook
 *
 * @example
 *   static bool my_error_hook(int type, int code, char const **message_out, struct ov_error *err) {
 *     if (type == MY_CUSTOM_ERROR_TYPE) {
 *       // For dynamic messages, use OV_ARRAY_GROW
 *       char *msg = NULL;
 *       if (!get_custom_error_message(code, &msg, err)) {
 *         return false;
 *       }
 *       *message_out = msg;
 *       return true;
 *     }
 *     // Override existing generic error with custom message
 *     if (type == ov_error_type_generic && code == ov_error_generic_not_found) {
 *       OV_ERROR_DEFINE(not_found_msg, ov_error_type_generic, ov_error_generic_not_found,
 *                       "Resource not found - check configuration");
 *       *message_out = not_found_msg.context;
 *       return true;
 *     }
 *     *message_out = NULL; // Use default handling
 *     return true;
 *   }
 *
 *   ov_error_set_autofill_hook(my_error_hook);
 */
void ov_error_set_autofill_hook(ov_error_autofill_hook_func hook_func);

/**
 * @brief Hook function type for custom error output
 *
 * This function type defines the signature for custom error output handlers,
 * completely replacing the default stderr output mechanism.
 *
 * @param str UTF-8 encoded string to output (always null-terminated)
 *
 * @note When this hook is set, it becomes the final output destination.
 *       No fallback to stderr occurs.
 */
typedef void (*ov_error_output_hook_func)(char const *str);

/**
 * @brief Set custom hook function for error output
 *
 * Sets a custom output handler that completely replaces the default stderr output.
 * This allows redirecting error messages to custom destinations like log files,
 * network services, or other output mechanisms.
 *
 * @param hook_func Function to handle error output, or NULL to restore default stderr output
 *
 * @note The hook function becomes the final output destination when set.
 *       No fallback to stderr occurs regardless of hook success/failure.
 */
void ov_error_set_output_hook(ov_error_output_hook_func hook_func);

/**
 * Automatically fills error message if not already set
 *
 * NOTE: You typically don't need to call this function directly. Most error handling functions
 * (like ov_error_report_and_destroy, ov_error_to_string) automatically generate messages when needed.
 *
 * This is a low-level helper function primarily useful when implementing custom
 * error message formatting or when you need to access the error message directly
 * before reporting. It retrieves an appropriate message based on the error type
 * and code only when the message field is currently NULL.
 *
 * @param target Pointer to error structure. If NULL or already has a message, no action is taken.
 * @param err Pointer to struct ov_error for error information.
 * @return true on success (including when no action needed), false on failure (check err_ptr for details)
 */
bool ov_error_autofill_message(struct ov_error_stack *const target, struct ov_error *const err);

bool ov_error_to_string(struct ov_error const *const src,
                        char **const dest,
                        bool const include_stack_trace,
                        struct ov_error *const err);

bool ov_error_report_and_destroy(struct ov_error *const target, char const *const message ERR_FILEPOS_PARAMS);

/**
 * Check if error matches specific type and code
 *
 * Searches through all active error stack entries (both fixed stack and extended stack)
 * to find a match for the specified type and code. An active stack entry is one where
 * the type is not ov_error_type_invalid.
 *
 * @param target Pointer to error structure to check. Must not be NULL (asserted in debug builds).
 *            If NULL is passed in release builds, returns true to make broken code fail explicitly.
 * @param type Expected error type (e.g., ov_error_type_generic)
 * @param code Expected error code (e.g., ov_error_generic_fail)
 * @return true if error matches both type and code in any active stack entry, false otherwise
 */
NODISCARD bool ov_error_is(struct ov_error const *const target, int const type, int const code);

/**
 * Get error code for the first matching error type
 *
 * Searches through all active error stack entries (both fixed stack and extended stack)
 * to find the first entry that matches the specified type and writes its code to the output parameter.
 *
 * @param target Pointer to error structure to search. Must not be NULL (asserted in debug builds).
 * @param type Error type to search for (e.g., ov_error_type_generic, ov_error_type_hresult)
 * @param code Pointer to write the error code to. Must not be NULL (asserted in debug builds).
 * @return true if matching entry found and code written, false if not found
 *
 * @example
 *   struct ov_error err = {0};
 *   OV_ERROR_SET_HRESULT(&err, E_FAIL);
 *   HRESULT hr;
 *   if (ov_error_get_code(&err, ov_error_type_hresult, (int*)&hr)) {
 *     // hr now contains E_FAIL
 *   }
 */
NODISCARD bool ov_error_get_code(struct ov_error const *const target, int const type, int *const code);

/**
 * @brief Destroy and clean up an error object
 *
 * Releases all memory associated with the error object and resets it to zero.
 *
 * @param err_ptr Pointer to struct ov_error to destroy. If NULL, the operation is silently ignored.
 *
 * @example
 *   struct ov_error err = {0};
 *   OV_ERROR_SET(&err, ov_error_type_generic, ov_error_generic_fail, "Error");
 *   OV_ERROR_DESTROY(&err); // err is now {0}
 */
#define OV_ERROR_DESTROY(err_ptr) (ov_error_destroy((err_ptr)MEM_FILEPOS_VALUES))

/**
 * @brief Define a complete static error with type, code, and message
 *
 * Creates a compile-time constant error structure with proper memory management headers.
 * Use this macro to define complete error information that can be safely used with
 * OV_ERROR_SET for optimal performance without memory allocation.
 *
 * @param name Variable name for the static error
 * @param type Error type (e.g., ov_error_type_generic)
 * @param code Error code (e.g., ov_error_generic_fail)
 * @param str String literal for the error message
 *
 * @see OV_ERROR_SET for usage example
 */
#define OV_ERROR_DEFINE(name, type, code, str)                                                                         \
  static struct __attribute__((__packed__)) {                                                                          \
    struct {                                                                                                           \
      size_t len;                                                                                                      \
      size_t cap;                                                                                                      \
    } header;                                                                                                          \
    char const buf[sizeof(str)];                                                                                       \
  } const name##_str = {{.len = sizeof(str) - 1}, str};                                                                \
  static struct ov_error_info const name = {                                                                           \
      type,                                                                                                            \
      code,                                                                                                            \
      name##_str.buf,                                                                                                  \
  }

/**
 * @brief Set error using predefined static error
 *
 * Use with OV_ERROR_DEFINE only for compile-time type safety.
 * This macro provides optimal performance with no memory allocation or string copying.
 *
 * @param err_ptr Pointer to struct ov_error. If NULL, the operation is silently ignored.
 * @param info_ptr Pointer to static error defined with OV_ERROR_DEFINE
 *
 * @example
 *   OV_ERROR_DEFINE(my_error, ov_error_type_generic, ov_error_generic_fail, "Something went wrong");
 *   OV_ERROR_SET(&err, &my_error);
 */
#define OV_ERROR_SET(err_ptr, info_ptr) (ov_error_set((err_ptr), (info_ptr)ERR_FILEPOS_VALUES))

/**
 * @brief Set generic error with message
 *
 * Convenience macro for setting generic type errors. Equivalent to
 * OV_ERROR_SET with ov_error_type_generic.
 *
 * @param err_ptr Pointer to struct ov_error. If NULL, the operation is silently ignored.
 * @param error_code Generic error code (e.g., ov_error_generic_fail)
 *
 * @example
 *   OV_ERROR_SET_GENERIC(&err, ov_error_generic_invalid_argument);
 */
#define OV_ERROR_SET_GENERIC(err_ptr, error_code)                                                                      \
  (ov_error_set((err_ptr),                                                                                             \
                &(struct ov_error_info const){                                                                         \
                    .type = ov_error_type_generic,                                                                     \
                    .code = (error_code),                                                                              \
                } ERR_FILEPOS_VALUES))

/**
 * @brief Set errno-based error
 *
 * Convenience macro for setting errno type errors. Equivalent to
 * OV_ERROR_SET with ov_error_type_errno.
 * The error message will be generated from the errno code when the error is displayed.
 *
 * @param err_ptr Pointer to struct ov_error. If NULL, the operation is silently ignored.
 * @param errno Standard errno value (e.g., ENOENT, ENOMEM)
 *
 * @example
 *   OV_ERROR_SET_ERRNO(&err, ENOENT);
 */
#define OV_ERROR_SET_ERRNO(err_ptr, errno)                                                                             \
  (ov_error_set((err_ptr),                                                                                             \
                &(struct ov_error_info const){                                                                         \
                    .type = ov_error_type_errno,                                                                       \
                    .code = (errno),                                                                                   \
                } ERR_FILEPOS_VALUES))

#ifdef _WIN32
/**
 * @brief Set Windows HRESULT-based error
 *
 * Convenience macro for setting HRESULT type errors on Windows. Equivalent to
 * OV_ERROR_SET with ov_error_type_hresult.
 * The error message will be generated from the HRESULT when the error is displayed.
 *
 * @param err_ptr Pointer to struct ov_error. If NULL, the operation is silently ignored.
 * @param hresult Windows HRESULT value
 *
 * @example
 *   HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
 *   OV_ERROR_SET_HRESULT(&err, hr);
 */
#  define OV_ERROR_SET_HRESULT(err_ptr, hresult)                                                                       \
    (ov_error_set((err_ptr),                                                                                           \
                  &(struct ov_error_info const){                                                                       \
                      .type = ov_error_type_hresult,                                                                   \
                      .code = (hresult),                                                                               \
                  } ERR_FILEPOS_VALUES))
#endif

/**
 * @brief Set error with formatted message
 *
 * Sets an error with a printf-style formatted message.
 *
 * @param err_ptr Pointer to struct ov_error. If NULL, the operation is silently ignored.
 * @param error_type Error type (e.g., ov_error_type_generic)
 * @param error_code Error code (e.g., ov_error_generic_fail)
 * @param reference Reference format string for translation safety, can be NULL. (e.g., "%1$s%2$d")
 * @param format Format string for message formatting (e.g., "File %1$s not found, error code %2$d")
 * @param ... Format arguments
 *
 * @example
 *   OV_ERROR_SETF(&err, ov_error_type_generic, ov_error_generic_fail,
 *                 "%1$s%2$d", "File %1$s not found, error code %2$d", "test.txt", 404);
 */
#define OV_ERROR_SETF(err_ptr, error_type, error_code, reference, format, ...)                                         \
  (ov_error_setf((err_ptr),                                                                                            \
                 (&(struct ov_error_info const){.type = (error_type), .code = (error_code), .context = (format)}),     \
                 (reference)ERR_FILEPOS_VALUES,                                                                        \
                 __VA_ARGS__))

/**
 * @brief Add error information to existing error
 *
 * Adds additional error information to an existing error for building
 * error chains. Allows adding meaningful context at each level of the call stack.
 *
 * @param err_ptr Pointer to struct ov_error with existing error. If NULL, the operation is silently ignored.
 * @param info_ptr Pointer to static error defined with OV_ERROR_DEFINE
 *
 * @example
 *   OV_ERROR_DEFINE(file_error, ov_error_type_generic, ov_error_generic_fail, "Failed to process file");
 *   if (!some_func(&err)) {
 *     OV_ERROR_PUSH(&err, &file_error); // Add context to existing error
 *     goto cleanup;
 *   }
 */
#define OV_ERROR_PUSH(err_ptr, info_ptr) (ov_error_push((err_ptr), (info_ptr)ERR_FILEPOS_VALUES))

/**
 * @brief Add formatted error information to existing error with printf-like formatting
 *
 * Similar to OV_ERROR_SETF but for adding error information to an existing error.
 * Allows printf-style formatted messages to be added to the error chain with custom type and code.
 *
 * @param err_ptr Pointer to existing error to add information to
 * @param error_type Error type (e.g., ov_error_type_generic)
 * @param error_code Error code (e.g., ov_error_generic_fail)
 * @param reference Reference parameter for error context
 * @param format Printf-style format string
 * @param ... Variable arguments for the format string
 *
 * @example
 *   if (!some_func(&err)) {
 *     OV_ERROR_PUSHF(&err, ov_error_type_generic, ov_error_generic_fail, NULL, "Failed at step %d", step_number);
 *     goto cleanup;
 *   }
 */
#define OV_ERROR_PUSHF(err_ptr, error_type, error_code, reference, format, ...)                                        \
  (ov_error_pushf((err_ptr),                                                                                           \
                  (&(struct ov_error_info const){.type = (error_type), .code = (error_code), .context = (format)}),    \
                  (reference)ERR_FILEPOS_VALUES,                                                                       \
                  __VA_ARGS__))
/**
 * @brief Add stack trace information to existing error
 *
 * Adds current file and line information to an existing error for
 * stack trace functionality. Used when passing errors up the call chain
 * without adding additional context.
 *
 * @param err_ptr Pointer to struct ov_error with existing error. If NULL, the operation is silently ignored.
 *
 * @example
 *   if (!some_func(&err)) {
 *     OV_ERROR_TRACE(&err); // Add current location to stack trace
 *     goto cleanup;
 *   }
 */
#define OV_ERROR_TRACE(err_ptr)                                                                                        \
  (ov_error_push((err_ptr), (&(struct ov_error_info const){.type = ov_error_type_trace}) ERR_FILEPOS_VALUES))

/**
 * @brief Add formatted stack trace information to existing error with printf-like formatting
 *
 * Similar to OV_ERROR_TRACE but allows adding formatted messages to the stack trace.
 * Used when you want to add contextual information to the trace without changing error type/code.
 *
 * @param err_ptr Pointer to existing error to add trace information to
 * @param reference Reference parameter for error context
 * @param format Printf-style format string
 * @param ... Variable arguments for the format string
 *
 * @example
 *   if (!some_func(&err)) {
 *     OV_ERROR_TRACEF(&err, NULL, "Processing item %d of %d", current, total);
 *     goto cleanup;
 *   }
 */
#define OV_ERROR_TRACEF(err_ptr, reference, format, ...)                                                               \
  (ov_error_pushf((err_ptr),                                                                                           \
                  (&(struct ov_error_info const){.type = ov_error_type_trace, .code = 0, .context = (format)}),        \
                  (reference)ERR_FILEPOS_VALUES,                                                                       \
                  __VA_ARGS__))

#define OV_ERROR_REPORT(err_ptr, msg) (ov_error_report_and_destroy((err_ptr), (msg)ERR_FILEPOS_VALUES))

// mem

#ifdef LEAK_DETECTOR
long ov_mem_get_allocated_count(void);
#endif

/**
 * @brief Low-level implementation for OV_REALLOC
 *
 * Use OV_REALLOC macro instead of calling this function directly.
 *
 * @param pp Pointer to the pointer to memory (will be updated)
 * @param n Number of items to allocate
 * @param item_size Size of each item in bytes
 * @param err Pointer to struct ov_error for error information
 * @return true on success, false on failure (check err for details)
 *
 * @see OV_REALLOC
 */
NODISCARD bool
ov_mem_realloc(void *const pp, size_t const n, size_t const item_size, struct ov_error *const err MEM_FILEPOS_PARAMS);

/**
 * @brief Low-level implementation for OV_FREE
 *
 * Use OV_FREE macro instead of calling this function directly.
 *
 * @param pp Pointer to the pointer to memory (will be set to NULL)
 *
 * @see OV_FREE
 */
void ov_mem_free(void *const pp MEM_FILEPOS_PARAMS);

/**
 * @brief Low-level implementation for OV_ALIGNED_ALLOC
 *
 * Use OV_ALIGNED_ALLOC macro instead of calling this function directly.
 *
 * @param pp Pointer to the pointer to memory (must be NULL initially, will be updated)
 * @param n Number of items to allocate
 * @param item_size Size of each item in bytes
 * @param align Alignment boundary (power of 2, max 256 bytes)
 * @param err Pointer to struct ov_error for error information
 * @return true on success, false on failure (check err for details)
 *
 * @see OV_ALIGNED_ALLOC
 */
NODISCARD bool ov_mem_aligned_alloc(void *const pp,
                                    size_t const n,
                                    size_t const item_size,
                                    size_t const align,
                                    struct ov_error *const err MEM_FILEPOS_PARAMS);

/**
 * @brief Low-level implementation for OV_ALIGNED_FREE
 *
 * Use OV_ALIGNED_FREE macro instead of calling this function directly.
 *
 * @param pp Pointer to the pointer to aligned memory (will be set to NULL)
 *
 * @see OV_ALIGNED_FREE
 */
void ov_mem_aligned_free(void *const pp MEM_FILEPOS_PARAMS);

/**
 * @brief Allocate, reallocate, or resize memory with new error system
 *
 * This macro provides a convenient interface to the ov_mem_realloc function,
 * automatically adding file position information for debugging and leak detection.
 * Functions like C standard library's realloc() but with enhanced error handling.
 *
 * @param pp Pointer to the pointer to memory (will be updated)
 * @param n Number of items to allocate
 * @param item_size Size of each item in bytes
 * @param err_ptr Pointer to struct ov_error for error information
 * @return true on success, false on failure (check err_ptr for details)
 *
 * @example
 *   void *ptr = NULL;
 *   struct ov_error err = {0};
 *
 *   // Allocate memory for 10 integers
 *   if (!OV_REALLOC(&ptr, 10, sizeof(int), &err)) {
 *     // Handle error
 *   }
 *
 *   // Resize to 20 integers
 *   if (!OV_REALLOC(&ptr, 20, sizeof(int), &err)) {
 *     // Handle error
 *   }
 */
#define OV_REALLOC(pp, n, item_size, err_ptr) (ov_mem_realloc((pp), (n), (item_size), (err_ptr)MEM_FILEPOS_VALUES))

/**
 * @brief Free memory allocated with OV_REALLOC
 *
 * This macro provides a convenient interface to the ov_mem_free function,
 * automatically adding file position information for debugging and leak detection.
 *
 * @param pp Pointer to the pointer to memory (will be set to NULL)
 *
 * @example
 *   void *ptr = NULL;
 *   struct ov_error err = {0};
 *
 *   // Allocate memory
 *   OV_REALLOC(&ptr, 10, sizeof(int), &err);
 *
 *   // Free memory
 *   OV_FREE(&ptr);  // ptr becomes NULL
 */
#define OV_FREE(pp) (ov_mem_free((pp)MEM_FILEPOS_VALUES))

/**
 * @brief Allocate aligned memory
 *
 * Allocates memory aligned to the specified boundary. The alignment must be a power of 2
 * and not exceed 256 bytes. When USE_MIMALLOC is enabled, uses mimalloc's aligned allocation.
 * Otherwise, uses a custom implementation that over-allocates and stores offset information.
 *
 * The pointer must initially be NULL. Memory allocated with this function must be freed
 * using OV_ALIGNED_FREE.
 *
 * @param pp Pointer to the pointer to memory (must be NULL initially, will be updated)
 * @param n Number of items to allocate
 * @param item_size Size of each item in bytes
 * @param align Alignment boundary (power of 2, max 256 bytes)
 * @param err Pointer to struct ov_error for error information
 * @return true on success, false on failure (check err for details)
 *
 * @example
 *   void *aligned_ptr = NULL;
 *   struct ov_error err = {0};
 *
 *   // Allocate 10 integers aligned to 32-byte boundary
 *   if (!OV_ALIGNED_ALLOC(&aligned_ptr, 10, sizeof(int), 32, &err)) {
 *     // Handle error
 *   }
 *
 *   // Use aligned_ptr...
 *
 *   OV_ALIGNED_FREE(&aligned_ptr);  // aligned_ptr becomes NULL
 */
#define OV_ALIGNED_ALLOC(pp, n, item_size, align, err)                                                                 \
  ov_mem_aligned_alloc((pp), (n), (item_size), (align), (err)MEM_FILEPOS_VALUES)

/**
 * @brief Free aligned memory allocated with OV_ALIGNED_ALLOC
 *
 * Frees memory that was allocated with OV_ALIGNED_ALLOC. This function properly handles
 * both mimalloc-based aligned allocations and custom aligned allocations by restoring
 * the original pointer before freeing. The pointer is automatically set to NULL after freeing.
 *
 * @param pp Pointer to the pointer to aligned memory (will be set to NULL)
 *
 * @example
 *   void *aligned_ptr = NULL;
 *   struct ov_error err = {0};
 *
 *   OV_ALIGNED_ALLOC(&aligned_ptr, 10, sizeof(int), 32, &err);
 *
 *   // Free the aligned memory
 *   OV_ALIGNED_FREE(&aligned_ptr);  // aligned_ptr is now NULL
 */
#define OV_ALIGNED_FREE(pp) ov_mem_aligned_free((pp)MEM_FILEPOS_VALUES)

/**
 * @brief Initialize ovbase library subsystems
 *
 * Initializes global subsystems required when using ov_rand_get_global_hint,
 * ALLOCATE_LOGGER, or LEAK_DETECTOR features. Must be called at program startup.
 *
 * @see ov_exit() Must be called before program termination
 */
void ov_init(void);

/**
 * @brief Cleanup ovbase library subsystems
 *
 * Performs cleanup and reports memory leaks/statistics when ALLOCATE_LOGGER
 * or LEAK_DETECTOR is enabled. Must be called before program termination.
 *
 * @see ov_init() Must be called at program startup
 */
void ov_exit(void);
