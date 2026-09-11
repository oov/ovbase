#include "codepoint_utf8.h"

static enum ov_codepoint_fn_result count_char32(int_fast32_t codepoint, void *ctx) {
  (void)codepoint;
  size_t *n = (size_t *)ctx;
  ++*n;
  return ov_codepoint_fn_result_continue;
}

size_t ov_utf8_to_char32_len(char const *const src, size_t const src_len) {
  size_t n = 0;
  if (!ov_utf8_to_codepoint(count_char32, &n, src, src_len)) {
    return 0;
  }
  return n;
}

struct char32_context {
  char32_t *cur;
  char32_t *end;
};

static enum ov_codepoint_fn_result write_char32(int_fast32_t codepoint, void *ctx) {
  struct char32_context *c = (struct char32_context *)ctx;
  if (c->end - c->cur <= 1) {
    return ov_codepoint_fn_result_abort;
  }
  *c->cur++ = (char32_t)codepoint;
  return ov_codepoint_fn_result_continue;
}

size_t ov_utf8_to_char32(
    char const *const src, size_t const src_len, char32_t *const dest, size_t const dest_len, size_t *const read) {
  if (!src || !src_len || !dest || !dest_len) {
    return 0;
  }
  struct char32_context ctx = {
      .cur = dest,
      .end = dest + dest_len,
  };
  size_t const r = ov_utf8_to_codepoint(write_char32, &ctx, src, src_len);
  *ctx.cur = L'\0';
  if (!r) {
    return 0;
  }
  if (read) {
    *read = r;
  }
  return (size_t)(ctx.cur - dest);
}
