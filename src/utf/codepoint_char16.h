#pragma once

#include <ovutf.h>

size_t ov_char16_to_codepoint(ov_codepoint_fn fn, void *ctx, char16_t const *const src, size_t const src_len);
