#pragma once

#include <ovbase.h>

bool ov_sprintf_char(
    char **const dest, char const *const reference, char const *const format, struct ov_error *const err, ...);

bool ov_vsprintf_char(char **const dest,
                      char const *const reference,
                      char const *const format,
                      struct ov_error *const err,
                      va_list valist);

bool ov_sprintf_wchar(
    wchar_t **const dest, wchar_t const *const reference, wchar_t const *const format, struct ov_error *const err, ...);

bool ov_vsprintf_wchar(wchar_t **const dest,
                       wchar_t const *const reference,
                       wchar_t const *const format,
                       struct ov_error *const err,
                       va_list valist);

bool ov_sprintf_append_char(
    char **const dest, char const *const reference, char const *const format, struct ov_error *const err, ...);

bool ov_vsprintf_append_char(char **const dest,
                             char const *const reference,
                             char const *const format,
                             struct ov_error *const err,
                             va_list valist);

bool ov_sprintf_append_wchar(
    wchar_t **const dest, wchar_t const *const reference, wchar_t const *const format, struct ov_error *const err, ...);

bool ov_vsprintf_append_wchar(wchar_t **const dest,
                              wchar_t const *const reference,
                              wchar_t const *const format,
                              struct ov_error *const err,
                              va_list valist);
