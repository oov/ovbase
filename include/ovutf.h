/**
 * @file ovutf.h
 * @brief converts UTF-8 to/from wchar_t
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

enum ov_codepoint_fn_result {
  ov_codepoint_fn_result_continue, /**< continue */
  ov_codepoint_fn_result_abort,    /**< treat the current code point as failure and abort processing */
  ov_codepoint_fn_result_finish,   /**< treat the current code point as success and abort processing */
};

typedef enum ov_codepoint_fn_result (*ov_codepoint_fn)(int_fast32_t codepoint, void *ctx);

/**
 * @brief Converts a UTF-8 string to a codepoint.
 *
 * @param fn Callback function
 * @param ctx Used in fn
 * @param src UTF-8 encoded string
 * @param src_len length of src
 * @return Returns the number of bytes processed on success, or zero on failure
 */
size_t ov_utf8_to_codepoint(ov_codepoint_fn fn, void *ctx, char const *const src, size_t const src_len);

/**
 * @brief Gets the number of characters when converted to wchar_t.
 *
 * @param src UTF-8 string
 * @param src_len length of src
 * @return Returns the number of characters in the wchar_t string on success, or zero on failure
 */
size_t ov_utf8_to_wchar_len(char const *const src, size_t const src_len);

/**
 * @brief Converts UTF-8 string to wchar_t string.
 *
 * @param src UTF-8 string
 * @param src_len length of src
 * @param dest destination buffer
 * @param dest_len length of dest
 * @param read Number of bytes read from src on success, can be NULL
 * @return Returns the number of characters in the wchar_t string on success, or zero on failure
 * @note If dest is not long enough, it is converted up to the middle. dest is always nul-terminated.
 */
size_t ov_utf8_to_wchar(
    char const *const src, size_t const src_len, wchar_t *const dest, size_t const dest_len, size_t *const read);

/**
 * @brief Converts a wchar_t string to a codepoint and calls a callback.
 *
 * @param fn Callback function called for each codepoint
 * @param ctx Used in callback
 * @param src wchar_t string
 * @param src_len length of src
 * @return Returns the number of characters processed on success, or zero on failure
 */
size_t ov_wchar_to_codepoint(ov_codepoint_fn fn, void *ctx, wchar_t const *const src, size_t const src_len);

/**
 * @brief Returns the number of characters when converted to UTF-8.
 *
 * @param src wchar_t string
 * @param src_len length of src
 * @return Returns the number of bytes in the UTF-8 string on success, or zero on failure
 */
size_t ov_wchar_to_utf8_len(wchar_t const *const src, size_t const src_len);

/**
 * @brief Converts wchar_t string and write to dest.
 *
 * @param src wchar_t string
 * @param src_len length of src
 * @param dest destination buffer
 * @param dest_len length of dest
 * @param read Number of characters read from src on success, can be NULL
 * @return Returns the number of characters in the wchar_t string on success, or zero on failure
 * @note If dest is not long enough, it is converted up to the middle. dest is always nul-terminated.
 */
size_t ov_wchar_to_utf8(
    wchar_t const *const src, size_t const src_len, char *const dest, size_t const dest_len, size_t *const read);

/**
 * @brief Converts a Shift_JIS string to a codepoint.
 *
 * @param fn Callback function
 * @param ctx Used in fn
 * @param src Shift_JIS encoded string
 * @param src_len length of src
 * @return Returns the number of bytes processed on success, or zero on failure
 */
size_t ov_sjis_to_codepoint(ov_codepoint_fn fn, void *ctx, char const *const src, size_t const src_len);

/**
 * @brief Returns the number of characters when converted to UTF-8.
 *
 * @param src char string
 * @param src_len length of src
 * @return Returns the number of bytes in the UTF-8 string on success, or zero on failure
 */
size_t ov_sjis_to_utf8_len(char const *const src, size_t const src_len);

/**
 * @brief Converts char string and write to dest.
 *
 * @param src char string
 * @param src_len length of src
 * @param dest destination buffer
 * @param dest_len length of dest
 * @param read Number of characters read from src on success, can be NULL
 * @return Returns the number of characters in the char string on success, or zero on failure
 * @note If dest is not long enough, it is converted up to the middle. dest is always nul-terminated.
 */
size_t ov_sjis_to_utf8(
    char const *const src, size_t const src_len, char *const dest, size_t const dest_len, size_t *const read);
