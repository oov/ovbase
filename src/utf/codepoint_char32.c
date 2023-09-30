#include "codepoint_char32.h"

#include <stdbool.h>

#include "common.h"

size_t ov_char32_to_codepoint(ov_codepoint_fn fn, void *ctx, char32_t const *const src, size_t const src_len) {
  size_t i = 0;
  while (i < src_len) {
    int_fast32_t const codepoint = (int_fast32_t)(src[i]);
    if (invalid_codepoint(codepoint)) {
      return 0;
    }
    enum ov_codepoint_fn_result r = fn(codepoint, ctx);
    if (r != ov_codepoint_fn_result_abort) {
      ++i;
    }
    if (r != ov_codepoint_fn_result_continue) {
      break;
    }
  }
  return i;
}
