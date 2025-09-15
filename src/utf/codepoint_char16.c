#include <ovutf.h>

#include <assert.h>
#include <stdbool.h>

#include "common.h"

static inline bool surrogate_high(int const ch) { return 0xd800 <= ch && ch < 0xdc00; }

static inline bool surrogate_low(int const ch) { return 0xdc00 <= ch && ch < 0xe000; }

size_t ov_char16_to_codepoint(ov_codepoint_fn fn, void *ctx, char16_t const *const src, size_t const src_len) {
  assert(fn != NULL && "fn must not be NULL");
  assert(src != NULL || src_len == 0 && "src must not be NULL when src_len > 0");
  if (!fn || (!src && src_len > 0)) {
    return 0;
  }
  size_t i = 0;
  while (i < src_len) {
    int_fast32_t codepoint = 0;
    if (surrogate_high(src[i])) {
      if (i + 2 > src_len || !surrogate_low(src[i + 1])) {
        return 0;
      }
      codepoint = 0x10000 + ((int_fast32_t)(src[i]) - 0xd800) * 0x400 + ((int_fast32_t)(src[i + 1]) - 0xdc00);
    } else {
      codepoint = (int_fast32_t)(src[i]);
    }
    if (invalid_codepoint(codepoint)) {
      return 0;
    }
    enum ov_codepoint_fn_result r = fn(codepoint, ctx);
    if (r != ov_codepoint_fn_result_abort) {
      i += (codepoint > 0xffff) ? 2 : 1;
    }
    if (r != ov_codepoint_fn_result_continue) {
      break;
    }
  }
  return i;
}
