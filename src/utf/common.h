#pragma once

#include <ovutf.h>

#include <stdbool.h>

enum ov_codepoint_fn_result ovutf_utf8_count(int_fast32_t codepoint, void *ctx);

struct ovutf_utf8_write_context {
  uint8_t *cur;
  uint8_t *end;
};

enum ov_codepoint_fn_result ovutf_utf8_write(int_fast32_t const codepoint, void *ctx);

static inline bool invalid_codepoint(int_fast32_t ch) {
  return ch < 0 || ch > 0x10ffff || (0xd800 <= ch && ch < 0xe000);
}
