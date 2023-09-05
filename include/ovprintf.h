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

int ov_vpprintf_char(void (*putc)(int c, void *ctx),
                     void *ctx,
                     char const *const reference,
                     char const *const format,
                     va_list valist) OV_PRINTF_ATTR(printf, 4, 0);
int ov_pprintf_char(void (*putc)(int c, void *ctx),
                    void *ctx,
                    char const *const reference,
                    char const *const format,
                    ...) OV_PRINTF_ATTR(printf, 4, 5);
int ov_snprintf_char(char *const dest, size_t destlen, char const *const reference, char const *const format, ...)
    OV_PRINTF_ATTR(printf, 4, 5);
int ov_vsnprintf_char(char *const dest,
                      size_t destlen,
                      char const *const reference,
                      char const *const format,
                      va_list valist) OV_PRINTF_ATTR(printf, 4, 0);

int ov_vpprintf_wchar(void (*putc)(int c, void *ctx),
                      void *ctx,
                      wchar_t const *const reference,
                      wchar_t const *const format,
                      va_list valist);
int ov_pprintf_wchar(
    void (*putc)(int c, void *ctx), void *ctx, wchar_t const *const reference, wchar_t const *const format, ...);
int ov_snprintf_wchar(
    wchar_t *const dest, size_t destlen, wchar_t const *const reference, wchar_t const *const format, ...);
int ov_vsnprintf_wchar(
    wchar_t *const dest, size_t destlen, wchar_t const *const reference, wchar_t const *const format, va_list valist);

int ov_printf_verify_format_char(char const *const reference, char const *const format);
int ov_printf_verify_format_wchar(wchar_t const *const reference, wchar_t const *const format);

#ifndef OV_GENERIC_CASE
#  define OV_GENERIC_CASE(typ, fn)                                                                                     \
  typ:                                                                                                                 \
    fn
#endif

#define ov_vpprintf(putc, ctx, reference, format, valist)                                                              \
  _Generic((format),                                                                                                   \
           OV_GENERIC_CASE(char const *, ov_vpprintf_char),                                                            \
           OV_GENERIC_CASE(char *, ov_vpprintf_char),                                                                  \
           OV_GENERIC_CASE(wchar_t const *, ov_vpprintf_wchar),                                                        \
           OV_GENERIC_CASE(wchar_t *, ov_vpprintf_wchar))((putc), (ctx), (reference), (format), (valist))
#define ov_pprintf(putc, ctx, reference, format, ...)                                                                  \
  _Generic((format),                                                                                                   \
           OV_GENERIC_CASE(char const *, ov_pprintf_char),                                                             \
           OV_GENERIC_CASE(char *, ov_pprintf_char),                                                                   \
           OV_GENERIC_CASE(wchar_t const *, ov_pprintf_wchar),                                                         \
           OV_GENERIC_CASE(wchar_t *, ov_pprintf_wchar))((putc), (ctx), (reference), (format), __VA_ARGS__)
#define ov_snprintf(char_ptr, destlen, reference, format, ...)                                                         \
  _Generic((format),                                                                                                   \
           OV_GENERIC_CASE(char const *, ov_snprintf_char),                                                            \
           OV_GENERIC_CASE(char *, ov_snprintf_char),                                                                  \
           OV_GENERIC_CASE(wchar_t const *, ov_snprintf_wchar),                                                        \
           OV_GENERIC_CASE(wchar_t *, ov_snprintf_wchar))((char_ptr), (destlen), (reference), (format), __VA_ARGS__)
#define ov_vsnprintf(char_ptr, destlen, reference, format, valist)                                                     \
  _Generic((format),                                                                                                   \
           OV_GENERIC_CASE(char const *, ov_vsnprintf_char),                                                           \
           OV_GENERIC_CASE(char *, ov_vsnprintf_char),                                                                 \
           OV_GENERIC_CASE(wchar_t const *, ov_vsnprintf_wchar),                                                       \
           OV_GENERIC_CASE(wchar_t *, ov_vsnprintf_wchar))((char_ptr), (destlen), (reference), (format), (valist))
#define ov_printf_verify_format(reference, format)                                                                     \
  _Generic((reference),                                                                                                \
           OV_GENERIC_CASE(char const *, ov_printf_verify_format_char),                                                \
           OV_GENERIC_CASE(char *, ov_printf_verify_format_char),                                                      \
           OV_GENERIC_CASE(wchar_t const *, ov_printf_verify_format_wchar),                                            \
           OV_GENERIC_CASE(wchar_t *, ov_printf_verify_format_wchar))((reference), (format))
