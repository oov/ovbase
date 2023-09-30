#pragma once

#include <ovutf.h>

size_t ov_wchar_to_codepoint(ov_codepoint_fn fn, void *ctx, wchar_t const *const src, size_t const src_len);
