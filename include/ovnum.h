#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>
#include <wchar.h>

/**
 * Converts a char string to a signed 64-bit integer.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atoi_char(char const *ptr, int64_t *const dest, bool const strict);

/**
 * Converts a char string to a double-precision floating-point number.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atof_char(char const *ptr, double *const dest, bool const strict);

/**
 * Converts a char string to an unsigned 64-bit integer.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atou_char(char const *ptr, uint64_t *const dest, bool const strict);

/**
 * Converts a signed 64-bit integer to a char string.
 * @param v Value to convert
 * @param buf Buffer to store the result (must not be NULL, size >= 32)
 * @return Pointer to the buffer on success
 */
char *ov_itoa_char(int64_t const v, char buf[32]);

/**
 * Converts an unsigned 64-bit integer to a char string.
 * @param v Value to convert
 * @param buf Buffer to store the result (must not be NULL, size >= 32)
 * @return Pointer to the buffer on success
 */
char *ov_utoa_char(uint64_t const v, char buf[32]);

/**
 * Converts a double-precision floating-point number to a char string.
 * @param d Value to convert
 * @param frac_len Number of decimal places (must be <= 30)
 * @param dot Decimal point character
 * @param buf Buffer to store the result (must not be NULL, size >= 64)
 * @return Pointer to the buffer on success
 */
char *ov_ftoa_char(double const d, size_t const frac_len, char const dot, char buf[64]);

/**
 * Converts a char16_t string to a signed 64-bit integer.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atoi_char16(char16_t const *ptr, int64_t *const dest, bool const strict);

/**
 * Converts a char16_t string to a double-precision floating-point number.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atof_char16(char16_t const *ptr, double *const dest, bool const strict);

/**
 * Converts a char16_t string to an unsigned 64-bit integer.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atou_char16(char16_t const *ptr, uint64_t *const dest, bool const strict);

/**
 * Converts a signed 64-bit integer to a char16_t string.
 * @param v Value to convert
 * @param buf Buffer to store the result (must not be NULL, size >= 32)
 * @return Pointer to the buffer on success
 */
char16_t *ov_itoa_char16(int64_t const v, char16_t buf[32]);

/**
 * Converts an unsigned 64-bit integer to a char16_t string.
 * @param v Value to convert
 * @param buf Buffer to store the result (must not be NULL, size >= 32)
 * @return Pointer to the buffer on success
 */
char16_t *ov_utoa_char16(uint64_t const v, char16_t buf[32]);

/**
 * Converts a double-precision floating-point number to a char16_t string.
 * @param d Value to convert
 * @param frac_len Number of decimal places (must be <= 30)
 * @param dot Decimal point character
 * @param buf Buffer to store the result (must not be NULL, size >= 64)
 * @return Pointer to the buffer on success
 */
char16_t *ov_ftoa_char16(double const d, size_t const frac_len, char16_t const dot, char16_t buf[64]);

/**
 * Converts a char32_t string to a signed 64-bit integer.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atoi_char32(char32_t const *ptr, int64_t *const dest, bool const strict);

/**
 * Converts a char32_t string to a double-precision floating-point number.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atof_char32(char32_t const *ptr, double *const dest, bool const strict);

/**
 * Converts a char32_t string to an unsigned 64-bit integer.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atou_char32(char32_t const *ptr, uint64_t *const dest, bool const strict);

/**
 * Converts a signed 64-bit integer to a char32_t string.
 * @param v Value to convert
 * @param buf Buffer to store the result (must not be NULL, size >= 32)
 * @return Pointer to the buffer on success
 */
char32_t *ov_itoa_char32(int64_t const v, char32_t buf[32]);

/**
 * Converts an unsigned 64-bit integer to a char32_t string.
 * @param v Value to convert
 * @param buf Buffer to store the result (must not be NULL, size >= 32)
 * @return Pointer to the buffer on success
 */
char32_t *ov_utoa_char32(uint64_t const v, char32_t buf[32]);

/**
 * Converts a double-precision floating-point number to a char32_t string.
 * @param d Value to convert
 * @param frac_len Number of decimal places (must be <= 30)
 * @param dot Decimal point character
 * @param buf Buffer to store the result (must not be NULL, size >= 64)
 * @return Pointer to the buffer on success
 */
char32_t *ov_ftoa_char32(double const d, size_t const frac_len, char32_t const dot, char32_t buf[64]);

/**
 * Converts a wchar_t string to a signed 64-bit integer.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atoi_wchar(wchar_t const *ptr, int64_t *const dest, bool const strict);

/**
 * Converts a wchar_t string to a double-precision floating-point number.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atof_wchar(wchar_t const *ptr, double *const dest, bool const strict);

/**
 * Converts a wchar_t string to an unsigned 64-bit integer.
 * @param ptr Source string to parse (must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
bool ov_atou_wchar(wchar_t const *ptr, uint64_t *const dest, bool const strict);

/**
 * Converts a signed 64-bit integer to a wchar_t string.
 * @param v Value to convert
 * @param buf Buffer to store the result (must not be NULL, size >= 32)
 * @return Pointer to the buffer on success
 */
wchar_t *ov_itoa_wchar(int64_t const v, wchar_t buf[32]);

/**
 * Converts an unsigned 64-bit integer to a wchar_t string.
 * @param v Value to convert
 * @param buf Buffer to store the result (must not be NULL, size >= 32)
 * @return Pointer to the buffer on success
 */
wchar_t *ov_utoa_wchar(uint64_t const v, wchar_t buf[32]);

/**
 * Converts a double-precision floating-point number to a wchar_t string.
 * @param d Value to convert
 * @param frac_len Number of decimal places (must be <= 30)
 * @param dot Decimal point character
 * @param buf Buffer to store the result (must not be NULL, size >= 64)
 * @return Pointer to the buffer on success
 */
wchar_t *ov_ftoa_wchar(double const d, size_t const frac_len, wchar_t const dot, wchar_t buf[64]);

/**
 * Generic macro to convert a string to a signed 64-bit integer.
 * Automatically selects the appropriate function based on the string type.
 * @param char_const_ptr Source string to parse (char* or wchar_t*, must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
#define OV_ATOI(char_const_ptr, dest, strict)                                                                          \
  _Generic((char_const_ptr),                                                                                           \
      char *: ov_atoi_char,                                                                                            \
      char const *: ov_atoi_char,                                                                                      \
      wchar_t *: ov_atoi_wchar,                                                                                        \
      wchar_t const *: ov_atoi_wchar)((char_const_ptr), (dest), (strict))

/**
 * Generic macro to convert a string to an unsigned 64-bit integer.
 * Automatically selects the appropriate function based on the string type.
 * @param char_const_ptr Source string to parse (char* or wchar_t*, must not be NULL)
 * @param dest Destination for the parsed value (must not be NULL)
 * @param strict If true, the entire string must be consumed for success
 * @return true on success, false on failure
 */
#define OV_ATOU(char_const_ptr, dest, strict)                                                                          \
  _Generic((char_const_ptr),                                                                                           \
      char *: ov_atou_char,                                                                                            \
      char const *: ov_atou_char,                                                                                      \
      wchar_t *: ov_atou_wchar,                                                                                        \
      wchar_t const *: ov_atou_wchar)((char_const_ptr), (dest), (strict))

/**
 * Generic macro to convert a signed 64-bit integer to a string.
 * Automatically selects the appropriate function based on the buffer type.
 * @param int64_t Value to convert
 * @param char_array_32 Buffer to store the result (char[32] or wchar_t[32], must not be NULL)
 * @return Pointer to the buffer on success
 */
#define OV_ITOA(int64_t, char_array_32)                                                                                \
  _Generic(&*(char_array_32),                                                                                          \
      char *: ov_itoa_char,                                                                                            \
      char const *: ov_itoa_char,                                                                                      \
      wchar_t *: ov_itoa_wchar,                                                                                        \
      wchar_t const *: ov_itoa_wchar)((int64_t), (char_array_32))

/**
 * Generic macro to convert an unsigned 64-bit integer to a string.
 * Automatically selects the appropriate function based on the buffer type.
 * @param uint64_t Value to convert
 * @param char_array_32 Buffer to store the result (char[32] or wchar_t[32], must not be NULL)
 * @return Pointer to the buffer on success
 */
#define OV_UTOA(uint64_t, char_array_32)                                                                               \
  _Generic(&*(char_array_32),                                                                                          \
      char *: ov_utoa_char,                                                                                            \
      char const *: ov_utoa_char,                                                                                      \
      wchar_t *: ov_utoa_wchar,                                                                                        \
      wchar_t const *: ov_utoa_wchar)((uint64_t), (char_array_32))

/**
 * Generic macro to convert a double-precision floating-point number to a string.
 * Automatically selects the appropriate function based on the buffer type.
 * @param double Value to convert
 * @param frac_len Number of decimal places (must be <= 30)
 * @param dot Decimal point character
 * @param char_array_64 Buffer to store the result (char[64] or wchar_t[64], must not be NULL)
 * @return Pointer to the buffer on success
 */
#define OV_FTOA(double, frac_len, dot, char_array_64)                                                                  \
  _Generic(&*(char_array_64),                                                                                          \
      char *: ov_ftoa_char,                                                                                            \
      char const *: ov_ftoa_char,                                                                                      \
      wchar_t *: ov_ftoa_wchar,                                                                                        \
      wchar_t const *: ov_ftoa_wchar)((double), (frac_len), (dot), (char_array_64))
