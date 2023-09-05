#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

bool ov_atoi_char(char const *ptr, int64_t *const dest, bool const strict);
bool ov_atou_char(char const *ptr, uint64_t *const dest, bool const strict);
char *ov_itoa_char(int64_t const v, char buf[32]);
char *ov_utoa_char(uint64_t const v, char buf[32]);
char *ov_ftoa_char(double const d, size_t const frac_len, char const dot, char buf[64]);

bool ov_atoi_wchar(wchar_t const *ptr, int64_t *const dest, bool const strict);
bool ov_atou_wchar(wchar_t const *ptr, uint64_t *const dest, bool const strict);
wchar_t *ov_itoa_wchar(int64_t const v, wchar_t buf[32]);
wchar_t *ov_utoa_wchar(uint64_t const v, wchar_t buf[32]);
wchar_t *ov_ftoa_wchar(double const d, size_t const frac_len, wchar_t const dot, wchar_t buf[64]);

#ifndef OV_GENERIC_CASE
#  define OV_GENERIC_CASE(typ, fn)                                                                                     \
  typ:                                                                                                                 \
    fn
#endif

#define ov_atoi(char_const_ptr, dest, strict)                                                                          \
  _Generic((char_const_ptr),                                                                                           \
           OV_GENERIC_CASE(char *, ov_atoi_char),                                                                      \
           OV_GENERIC_CASE(char const *, ov_atoi_char),                                                                \
           OV_GENERIC_CASE(wchar_t *, ov_atoi_wchar),                                                                  \
           OV_GENERIC_CASE(wchar_t const *, ov_atoi_wchar))((char_const_ptr), (dest), (strict))
#define ov_atou(char_const_ptr, dest, strict)                                                                          \
  _Generic((char_const_ptr),                                                                                           \
           OV_GENERIC_CASE(char *, ov_atou_char),                                                                      \
           OV_GENERIC_CASE(char const *, ov_atou_char),                                                                \
           OV_GENERIC_CASE(wchar_t *, ov_atou_wchar),                                                                  \
           OV_GENERIC_CASE(wchar_t const *, ov_atou_wchar))((char_const_ptr), (dest), (strict))

#define ov_itoa(int64_t, char_array_32)                                                                                \
  _Generic(&*(char_array_32),                                                                                          \
           OV_GENERIC_CASE(char *, ov_itoa_char),                                                                      \
           OV_GENERIC_CASE(char const *, ov_itoa_char),                                                                \
           OV_GENERIC_CASE(wchar_t *, ov_itoa_wchar),                                                                  \
           OV_GENERIC_CASE(wchar_t const *, ov_itoa_wchar))((int64_t), (char_array_32))

#define ov_utoa(uint64_t, char_array_32)                                                                               \
  _Generic(&*(char_array_32),                                                                                          \
           OV_GENERIC_CASE(char *, ov_utoa_char),                                                                      \
           OV_GENERIC_CASE(char const *, ov_utoa_char),                                                                \
           OV_GENERIC_CASE(wchar_t *, ov_utoa_wchar),                                                                  \
           OV_GENERIC_CASE(wchar_t const *, ov_utoa_wchar))((uint64_t), (char_array_32))

#define ov_ftoa(double, frac_len, dot, char_array_64)                                                                  \
  _Generic(&*(char_array_64),                                                                                          \
           OV_GENERIC_CASE(char *, ov_ftoa_char),                                                                      \
           OV_GENERIC_CASE(char const *, ov_ftoa_char),                                                                \
           OV_GENERIC_CASE(wchar_t *, ov_ftoa_wchar),                                                                  \
           OV_GENERIC_CASE(wchar_t const *, ov_ftoa_wchar))((double), (frac_len), (dot), (char_array_64))
