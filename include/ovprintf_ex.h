#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <wchar.h>

struct ov_error;

/**
 * @brief Format a string into a dynamically allocated char buffer
 *
 * Formats a string using printf-style formatting and stores the result in a dynamically
 * allocated buffer. The buffer is automatically allocated or reallocated as needed.
 * If *dest is not NULL, its existing content will be cleared before formatting.
 *
 * @param dest Pointer to char* buffer. Must not be NULL. Will be allocated/reallocated automatically.
 *             Must be freed using OV_ARRAY_DESTROY() from ovarray.h
 * @param err Pointer to error structure for detailed error information. Can be NULL.
 * @param reference Reference format string for type safety (can be NULL). Should specify the
 *                  correct argument types and order to prevent crashes from format string errors
 * @param format Printf-style format string. Must not be NULL.
 * @param ... Variable arguments for formatting
 * @return true on success, false on failure
 *
 * @note The dest buffer uses ovarray.h dynamic array system. Use OV_ARRAY_DESTROY(dest)
 *       to free the memory when no longer needed.
 * @note If dest points to an existing buffer, it will be reused and resized as needed.
 */
bool ov_sprintf_char(
    char **const dest, struct ov_error *const err, char const *const reference, char const *const format, ...);

/**
 * @brief Format a string into a dynamically allocated char buffer (va_list version)
 *
 * Same as ov_sprintf_char() but takes a va_list instead of variable arguments.
 *
 * @param dest Pointer to char* buffer. Must not be NULL. Will be allocated/reallocated automatically.
 *             Must be freed using OV_ARRAY_DESTROY() from ovarray.h
 * @param err Pointer to error structure for detailed error information. Can be NULL.
 * @param reference Reference format string for type safety (can be NULL). Should specify the
 *                  correct argument types and order to prevent crashes from format string errors
 * @param format Printf-style format string. Must not be NULL.
 * @param valist Variable arguments list
 * @return true on success, false on failure
 *
 * @note The dest buffer uses ovarray.h dynamic array system. Use OV_ARRAY_DESTROY(dest)
 *       to free the memory when no longer needed.
 */
bool ov_vsprintf_char(char **const dest,
                      struct ov_error *const err,
                      char const *const reference,
                      char const *const format,
                      va_list valist);

/**
 * @brief Format a string into a dynamically allocated wchar_t buffer
 *
 * Formats a string using printf-style formatting and stores the result in a dynamically
 * allocated wide character buffer. The buffer is automatically allocated or reallocated as needed.
 * If *dest is not NULL, its existing content will be cleared before formatting.
 *
 * @param dest Pointer to wchar_t* buffer. Must not be NULL. Will be allocated/reallocated automatically.
 *             Must be freed using OV_ARRAY_DESTROY() from ovarray.h
 * @param err Pointer to error structure for detailed error information. Can be NULL.
 * @param reference Reference wide format string for type safety (can be NULL). Should specify the
 *                  correct argument types and order to prevent crashes from format string errors
 * @param format Printf-style format wide string. Must not be NULL.
 * @param ... Variable arguments for formatting
 * @return true on success, false on failure
 *
 * @note The dest buffer uses ovarray.h dynamic array system. Use OV_ARRAY_DESTROY(dest)
 *       to free the memory when no longer needed.
 * @note If dest points to an existing buffer, it will be reused and resized as needed.
 */
bool ov_sprintf_wchar(
    wchar_t **const dest, struct ov_error *const err, wchar_t const *const reference, wchar_t const *const format, ...);

/**
 * @brief Format a string into a dynamically allocated wchar_t buffer (va_list version)
 *
 * Same as ov_sprintf_wchar() but takes a va_list instead of variable arguments.
 *
 * @param dest Pointer to wchar_t* buffer. Must not be NULL. Will be allocated/reallocated automatically.
 *             Must be freed using OV_ARRAY_DESTROY() from ovarray.h
 * @param err Pointer to error structure for detailed error information. Can be NULL.
 * @param reference Reference wide format string for type safety (can be NULL). Should specify the
 *                  correct argument types and order to prevent crashes from format string errors
 * @param format Printf-style format wide string. Must not be NULL.
 * @param valist Variable arguments list
 * @return true on success, false on failure
 *
 * @note The dest buffer uses ovarray.h dynamic array system. Use OV_ARRAY_DESTROY(dest)
 *       to free the memory when no longer needed.
 */
bool ov_vsprintf_wchar(wchar_t **const dest,
                       struct ov_error *const err,
                       wchar_t const *const reference,
                       wchar_t const *const format,
                       va_list valist);

/**
 * @brief Append formatted string to an existing dynamically allocated char buffer
 *
 * Formats a string using printf-style formatting and appends the result to an existing
 * buffer. If *dest is NULL, this function behaves like ov_sprintf_char().
 * The buffer is automatically reallocated as needed to accommodate the new content.
 *
 * @param dest Pointer to char* buffer. Must not be NULL. Will be allocated/reallocated automatically.
 *             Must be freed using OV_ARRAY_DESTROY() from ovarray.h
 * @param err Pointer to error structure for detailed error information. Can be NULL.
 * @param reference Reference format string for type safety (can be NULL). Should specify the
 *                  correct argument types and order to prevent crashes from format string errors
 * @param format Printf-style format string. Must not be NULL.
 * @param ... Variable arguments for formatting
 * @return true on success, false on failure
 *
 * @note The dest buffer uses ovarray.h dynamic array system. Use OV_ARRAY_DESTROY(dest)
 *       to free the memory when no longer needed.
 * @note This function preserves existing content and appends new content to the end.
 * @note On failure, existing content in dest is preserved.
 */
bool ov_sprintf_append_char(
    char **const dest, struct ov_error *const err, char const *const reference, char const *const format, ...);

/**
 * @brief Append formatted string to an existing dynamically allocated char buffer (va_list version)
 *
 * Same as ov_sprintf_append_char() but takes a va_list instead of variable arguments.
 *
 * @param dest Pointer to char* buffer. Must not be NULL. Will be allocated/reallocated automatically.
 *             Must be freed using OV_ARRAY_DESTROY() from ovarray.h
 * @param err Pointer to error structure for detailed error information. Can be NULL.
 * @param reference Reference format string for type safety (can be NULL). Should specify the
 *                  correct argument types and order to prevent crashes from format string errors
 * @param format Printf-style format string. Must not be NULL.
 * @param valist Variable arguments list
 * @return true on success, false on failure
 *
 * @note The dest buffer uses ovarray.h dynamic array system. Use OV_ARRAY_DESTROY(dest)
 *       to free the memory when no longer needed.
 * @note This function preserves existing content and appends new content to the end.
 * @note On failure, existing content in dest is preserved.
 */
bool ov_vsprintf_append_char(char **const dest,
                             struct ov_error *const err,
                             char const *const reference,
                             char const *const format,
                             va_list valist);

/**
 * @brief Append formatted string to an existing dynamically allocated wchar_t buffer
 *
 * Formats a string using printf-style formatting and appends the result to an existing
 * wide character buffer. If *dest is NULL, this function behaves like ov_sprintf_wchar().
 * The buffer is automatically reallocated as needed to accommodate the new content.
 *
 * @param dest Pointer to wchar_t* buffer. Must not be NULL. Will be allocated/reallocated automatically.
 *             Must be freed using OV_ARRAY_DESTROY() from ovarray.h
 * @param err Pointer to error structure for detailed error information. Can be NULL.
 * @param reference Reference wide format string for type safety (can be NULL). Should specify the
 *                  correct argument types and order to prevent crashes from format string errors
 * @param format Printf-style format wide string. Must not be NULL.
 * @param ... Variable arguments for formatting
 * @return true on success, false on failure
 *
 * @note The dest buffer uses ovarray.h dynamic array system. Use OV_ARRAY_DESTROY(dest)
 *       to free the memory when no longer needed.
 * @note This function preserves existing content and appends new content to the end.
 * @note On failure, existing content in dest is preserved.
 */
bool ov_sprintf_append_wchar(
    wchar_t **const dest, struct ov_error *const err, wchar_t const *const reference, wchar_t const *const format, ...);

/**
 * @brief Append formatted string to an existing dynamically allocated wchar_t buffer (va_list version)
 *
 * Same as ov_sprintf_append_wchar() but takes a va_list instead of variable arguments.
 *
 * @param dest Pointer to wchar_t* buffer. Must not be NULL. Will be allocated/reallocated automatically.
 *             Must be freed using OV_ARRAY_DESTROY() from ovarray.h
 * @param err Pointer to error structure for detailed error information. Can be NULL.
 * @param reference Reference wide format string for type safety (can be NULL). Should specify the
 *                  correct argument types and order to prevent crashes from format string errors
 * @param format Printf-style format wide string. Must not be NULL.
 * @param valist Variable arguments list
 * @return true on success, false on failure
 *
 * @note The dest buffer uses ovarray.h dynamic array system. Use OV_ARRAY_DESTROY(dest)
 *       to free the memory when no longer needed.
 * @note This function preserves existing content and appends new content to the end.
 * @note On failure, existing content in dest is preserved.
 */
bool ov_vsprintf_append_wchar(wchar_t **const dest,
                              struct ov_error *const err,
                              wchar_t const *const reference,
                              wchar_t const *const format,
                              va_list valist);
