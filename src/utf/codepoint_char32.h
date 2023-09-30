#pragma once

#include <ovutf.h>

size_t ov_char32_to_codepoint(ov_codepoint_fn fn, void *ctx, char32_t const *const src, size_t const src_len);
