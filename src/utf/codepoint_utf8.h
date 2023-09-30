#pragma once

#include <ovutf.h>

size_t ov_utf8_to_codepoint(ov_codepoint_fn fn, void *ctx, char const *const src, size_t const src_len);
