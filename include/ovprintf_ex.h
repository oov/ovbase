#pragma once

#include <ovbase.h>

NODISCARD error ov_sprintf_char(char **const dest, char const *const reference, char const *const format, ...);

NODISCARD error ov_vsprintf_char(char **const dest,
                                 char const *const reference,
                                 char const *const format,
                                 va_list valist);

NODISCARD error ov_sprintf_wchar(wchar_t **const dest,
                                 wchar_t const *const reference,
                                 wchar_t const *const format,
                                 ...);

NODISCARD error ov_vsprintf_wchar(wchar_t **const dest,
                                  wchar_t const *const reference,
                                  wchar_t const *const format,
                                  va_list valist);
