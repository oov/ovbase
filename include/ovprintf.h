#pragma once

#include <stdarg.h>
#include <stddef.h>

#ifndef OV_PRINTF_ATTR
#  if 0
format annotation does not support '%1$s'
#    ifdef __GNUC__
#      define OV_PRINTF_ATTR(FUNC, FORMAT, VARGS) __attribute__((format(FUNC, FORMAT, VARGS)))
#    else
#      define OV_PRINTF_ATTR(FUNC, FORMAT, VARGS)
#    endif
#  else
#    define OV_PRINTF_ATTR(FUNC, FORMAT, VARGS)
#  endif
#endif

/**
 * @brief Print formatted output using a character callback function (char version)
 *
 * Formats a string according to the format specification and calls putc for each character.
 * The reference parameter provides translation safety when format comes from external sources.
 *
 * @param putc Output function called for each character. Must not be NULL.
 * @param ctx Context pointer passed to putc callback. Can be NULL.
 * @param reference Reference format string for translation safety. Can be NULL.
 * @param format Format string for output. Must not be NULL.
 * @param valist Variable argument list containing format arguments.
 * @return Number of characters that would be written (excluding null terminator), or -1 on error.
 *
 * @example
 *   ov_vpprintf_char(my_putc, ctx, "%1$s%2$d", gettext("File %1$s error %2$d"), valist);
 */
int ov_vpprintf_char(void (*putc)(int c, void *ctx),
                     void *ctx,
                     char const *const reference,
                     char const *const format,
                     va_list valist) OV_PRINTF_ATTR(printf, 4, 0);

/**
 * @brief Print formatted output using a character callback function (char version)
 *
 * Formats a string according to the format specification and calls putc for each character.
 * The reference parameter provides translation safety when format comes from external sources.
 *
 * @param putc Output function called for each character. Must not be NULL.
 * @param ctx Context pointer passed to putc callback. Can be NULL.
 * @param reference Reference format string for translation safety. Can be NULL.
 * @param format Format string for output. Must not be NULL.
 * @param ... Format arguments.
 * @return Number of characters that would be written (excluding null terminator), or -1 on error.
 *
 * @example
 *   ov_pprintf_char(my_putc, ctx, "%1$s%2$d", gettext("File %1$s error %2$d"), filename, errno);
 */
int ov_pprintf_char(void (*putc)(int c, void *ctx),
                    void *ctx,
                    char const *const reference,
                    char const *const format,
                    ...) OV_PRINTF_ATTR(printf, 4, 5);

/**
 * @brief Format string into a buffer with size limit (char version)
 *
 * Formats a string according to the format specification into the destination buffer.
 * The reference parameter provides translation safety when format comes from external sources.
 *
 * @param dest Destination buffer. Can be NULL (for size calculation only).
 * @param destlen Size of destination buffer in characters. Ignored if dest is NULL.
 * @param reference Reference format string for translation safety. Can be NULL.
 * @param format Format string for output. Must not be NULL.
 * @param ... Format arguments.
 * @return Number of characters that would be written (excluding null terminator), or -1 on error.
 *
 * @example
 *   ov_snprintf_char(buffer, sizeof(buffer), "%1$s%2$d", gettext("File %1$s error %2$d"), filename, errno);
 */
int ov_snprintf_char(char *const dest, size_t destlen, char const *const reference, char const *const format, ...)
    OV_PRINTF_ATTR(printf, 4, 5);

/**
 * @brief Format string into a buffer with size limit (char version)
 *
 * Formats a string according to the format specification into the destination buffer.
 * The reference parameter provides translation safety when format comes from external sources.
 *
 * @param dest Destination buffer. Can be NULL (for size calculation only).
 * @param destlen Size of destination buffer in characters. Ignored if dest is NULL.
 * @param reference Reference format string for translation safety. Can be NULL.
 * @param format Format string for output. Must not be NULL.
 * @param valist Variable argument list containing format arguments.
 * @return Number of characters that would be written (excluding null terminator), or -1 on error.
 *
 * @example
 *   ov_vsnprintf_char(buffer, sizeof(buffer), "%1$s%2$d", gettext("File %1$s error %2$d"), valist);
 */
int ov_vsnprintf_char(char *const dest,
                      size_t destlen,
                      char const *const reference,
                      char const *const format,
                      va_list valist) OV_PRINTF_ATTR(printf, 4, 0);

/**
 * @brief Print formatted output using a character callback function (wchar_t version)
 *
 * Formats a wide string according to the format specification and calls putc for each character.
 * The reference parameter provides translation safety when format comes from external sources.
 *
 * @param putc Output function called for each character. Must not be NULL.
 * @param ctx Context pointer passed to putc callback. Can be NULL.
 * @param reference Reference format string for translation safety. Can be NULL.
 * @param format Format string for output. Must not be NULL.
 * @param valist Variable argument list containing format arguments.
 * @return Number of characters that would be written (excluding null terminator), or -1 on error.
 *
 * @example
 *   ov_vpprintf_wchar(my_putc, ctx, L"%1$s%2$d", gettext_wide(L"File %1$s error %2$d"), valist);
 */
int ov_vpprintf_wchar(void (*putc)(int c, void *ctx),
                      void *ctx,
                      wchar_t const *const reference,
                      wchar_t const *const format,
                      va_list valist);

/**
 * @brief Print formatted output using a character callback function (wchar_t version)
 *
 * Formats a wide string according to the format specification and calls putc for each character.
 * The reference parameter provides translation safety when format comes from external sources.
 *
 * @param putc Output function called for each character. Must not be NULL.
 * @param ctx Context pointer passed to putc callback. Can be NULL.
 * @param reference Reference format string for translation safety. Can be NULL.
 * @param format Format string for output. Must not be NULL.
 * @param ... Format arguments.
 * @return Number of characters that would be written (excluding null terminator), or -1 on error.
 *
 * @example
 *   ov_pprintf_wchar(my_putc, ctx, L"%1$s%2$d", gettext_wide(L"File %1$s error %2$d"), filename, errno);
 */
int ov_pprintf_wchar(
    void (*putc)(int c, void *ctx), void *ctx, wchar_t const *const reference, wchar_t const *const format, ...);

/**
 * @brief Format wide string into a buffer with size limit (wchar_t version)
 *
 * Formats a wide string according to the format specification into the destination buffer.
 * The reference parameter provides translation safety when format comes from external sources.
 *
 * @param dest Destination buffer. Can be NULL (for size calculation only).
 * @param destlen Size of destination buffer in characters. Ignored if dest is NULL.
 * @param reference Reference format string for translation safety. Can be NULL.
 * @param format Format string for output. Must not be NULL.
 * @param ... Format arguments.
 * @return Number of characters that would be written (excluding null terminator), or -1 on error.
 *
 * @example
 *   ov_snprintf_wchar(buffer, buffer_size, L"%1$s%2$d", gettext_wide(L"File %1$s error %2$d"), filename, errno);
 */
int ov_snprintf_wchar(
    wchar_t *const dest, size_t destlen, wchar_t const *const reference, wchar_t const *const format, ...);

/**
 * @brief Format wide string into a buffer with size limit (wchar_t version)
 *
 * Formats a wide string according to the format specification into the destination buffer.
 * The reference parameter provides translation safety when format comes from external sources.
 *
 * @param dest Destination buffer. Can be NULL (for size calculation only).
 * @param destlen Size of destination buffer in characters. Ignored if dest is NULL.
 * @param reference Reference format string for translation safety. Can be NULL.
 * @param format Format string for output. Must not be NULL.
 * @param valist Variable argument list containing format arguments.
 * @return Number of characters that would be written (excluding null terminator), or -1 on error.
 *
 * @example
 *   ov_vsnprintf_wchar(buffer, buffer_size, L"%1$s%2$d", gettext_wide(L"File %1$s error %2$d"), valist);
 */
int ov_vsnprintf_wchar(
    wchar_t *const dest, size_t destlen, wchar_t const *const reference, wchar_t const *const format, va_list valist);

/**
 * @brief Verify format string safety against reference (char version)
 *
 * Verifies that the format string is compatible with the reference format string
 * to prevent crashes from incorrect argument types or order in translated strings.
 *
 * @param reference Reference format string for validation. Can be NULL.
 * @param format Format string to validate. Can be NULL.
 * @return Non-zero if format is safe to use with reference arguments, 0 otherwise.
 *
 * @example
 *   if (ov_printf_verify_format_char("%1$s%2$d", translated_format)) {
 *     // Safe to use translated_format
 *   }
 */
int ov_printf_verify_format_char(char const *const reference, char const *const format);

/**
 * @brief Verify format string safety against reference (wchar_t version)
 *
 * Verifies that the wide format string is compatible with the reference format string
 * to prevent crashes from incorrect argument types or order in translated strings.
 *
 * @param reference Reference format string for validation. Can be NULL.
 * @param format Format string to validate. Can be NULL.
 * @return Non-zero if format is safe to use with reference arguments, 0 otherwise.
 *
 * @example
 *   if (ov_printf_verify_format_wchar(L"%1$s%2$d", translated_format)) {
 *     // Safe to use translated_format
 *   }
 */
int ov_printf_verify_format_wchar(wchar_t const *const reference, wchar_t const *const format);

#define OV_VPPRINTF(putc, ctx, reference, format, valist)                                                              \
  _Generic((format),                                                                                                   \
      char const *: ov_vpprintf_char,                                                                                  \
      char *: ov_vpprintf_char,                                                                                        \
      wchar_t const *: ov_vpprintf_wchar,                                                                              \
      wchar_t *: ov_vpprintf_wchar)((putc), (ctx), (reference), (format), (valist))

#define OV_PPRINTF(putc, ctx, reference, format, ...)                                                                  \
  _Generic((format),                                                                                                   \
      char const *: ov_pprintf_char,                                                                                   \
      char *: ov_pprintf_char,                                                                                         \
      wchar_t const *: ov_pprintf_wchar,                                                                               \
      wchar_t *: ov_pprintf_wchar)((putc), (ctx), (reference), (format), __VA_ARGS__)

#define OV_SNPRINTF(char_ptr, destlen, reference, format, ...)                                                         \
  _Generic((format),                                                                                                   \
      char const *: ov_snprintf_char,                                                                                  \
      char *: ov_snprintf_char,                                                                                        \
      wchar_t const *: ov_snprintf_wchar,                                                                              \
      wchar_t *: ov_snprintf_wchar)((char_ptr), (destlen), (reference), (format), __VA_ARGS__)

#define OV_VSNPRINTF(char_ptr, destlen, reference, format, valist)                                                     \
  _Generic((format),                                                                                                   \
      char const *: ov_vsnprintf_char,                                                                                 \
      char *: ov_vsnprintf_char,                                                                                       \
      wchar_t const *: ov_vsnprintf_wchar,                                                                             \
      wchar_t *: ov_vsnprintf_wchar)((char_ptr), (destlen), (reference), (format), (valist))

#define OV_PRINTF_VERIFY_FORMAT(reference, format)                                                                     \
  _Generic((reference),                                                                                                \
      char const *: ov_printf_verify_format_char,                                                                      \
      char *: ov_printf_verify_format_char,                                                                            \
      wchar_t const *: ov_printf_verify_format_wchar,                                                                  \
      wchar_t *: ov_printf_verify_format_wchar)((reference), (format))
