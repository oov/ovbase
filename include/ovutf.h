#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <wchar.h>

bool utf8_to_wchar(char const *const src,
                   size_t const src_len,
                   wchar_t *const dest,
                   size_t const dest_len,
                   size_t *const read,
                   size_t *const written);

bool utf8_to_wchar_len(char const *const src, size_t const src_len, size_t *const len);
