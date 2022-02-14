#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

bool ovbase_atoi_char(char const *ptr, int64_t *const dest, bool const strict);
bool ovbase_atou_char(char const *ptr, uint64_t *const dest, bool const strict);
char *ovbase_itoa_char(int64_t const v, char buf[32]);
char *ovbase_utoa_char(uint64_t const v, char buf[32]);
char *ovbase_ftoa_char(double const d, size_t const frac_len, char const dot, char buf[64]);

bool ovbase_atoi_wchar(wchar_t const *ptr, int64_t *const dest, bool const strict);
bool ovbase_atou_wchar(wchar_t const *ptr, uint64_t *const dest, bool const strict);
wchar_t *ovbase_itoa_wchar(int64_t const v, wchar_t buf[32]);
wchar_t *ovbase_utoa_wchar(uint64_t const v, wchar_t buf[32]);
wchar_t *ovbase_ftoa_wchar(double const d, size_t const frac_len, wchar_t const dot, wchar_t buf[64]);
