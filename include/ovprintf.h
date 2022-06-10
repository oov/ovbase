#pragma once

#include <stdarg.h>
#include <stddef.h>

#ifndef OV_PRINTF_ATTR
#  ifdef __GNUC__
#    define OV_PRINTF_ATTR(FUNC, FORMAT, VARGS) __attribute__((format(FUNC, FORMAT, VARGS)))
#  else
#    define OV_PRINTF_ATTR(FUNC, FORMAT, VARGS)
#  endif
#endif

int ov_snprintf_char(char *const dest, size_t destlen, char const *const format, ...) OV_PRINTF_ATTR(printf, 3, 4);
int ov_vsnprintf_char(char *const dest, size_t destlen, char const *const format, va_list valist)
    OV_PRINTF_ATTR(printf, 3, 0);
int ov_snprintf_wchar(wchar_t *const dest, size_t destlen, wchar_t const *const format, ...);
int ov_vsnprintf_wchar(wchar_t *const dest, size_t destlen, wchar_t const *const format, va_list valist);

#ifndef OV_GENERIC_CASE
#  define OV_GENERIC_CASE(typ, fn)                                                                                     \
  typ:                                                                                                                 \
    fn
#endif

#define ov_snprintf(char_ptr, destlen, format, ...)                                                                    \
  _Generic((format),                                                                                                   \
           OV_GENERIC_CASE(char const *, ov_snprintf_char),                                                            \
           OV_GENERIC_CASE(char const *const, ov_snprintf_char),                                                       \
           OV_GENERIC_CASE(char *, ov_snprintf_char),                                                                  \
           OV_GENERIC_CASE(char *const, ov_snprintf_char),                                                             \
           OV_GENERIC_CASE(wchar_t const *, ov_snprintf_wchar),                                                        \
           OV_GENERIC_CASE(wchar_t const *const, ov_snprintf_wchar),                                                   \
           OV_GENERIC_CASE(wchar_t *, ov_snprintf_wchar),                                                              \
           OV_GENERIC_CASE(wchar_t *const, ov_snprintf_wchar))((char_ptr), (destlen), (format), __VA_ARGS__)
#define ov_vsnprintf(char_ptr, destlen, format, valist)                                                                \
  _Generic((format),                                                                                                   \
           OV_GENERIC_CASE(char const *, ov_vsnprintf_char),                                                           \
           OV_GENERIC_CASE(char const *const, ov_vsnprintf_char),                                                      \
           OV_GENERIC_CASE(char *, ov_vsnprintf_char),                                                                 \
           OV_GENERIC_CASE(char *const, ov_vsnprintf_char),                                                            \
           OV_GENERIC_CASE(wchar_t const *, ov_vsnprintf_wchar),                                                       \
           OV_GENERIC_CASE(wchar_t const *const, ov_vsnprintf_wchar),                                                  \
           OV_GENERIC_CASE(wchar_t *, ov_vsnprintf_wchar),                                                             \
           OV_GENERIC_CASE(wchar_t *const, ov_vsnprintf_wchar))((char_ptr), (destlen), (format), (valist))
