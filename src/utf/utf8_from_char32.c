#include "codepoint_char32.h"
#include "common.h"

size_t ov_char32_to_utf8_len(char32_t const *const src, size_t const src_len) {
  size_t n = 0;
  if (!ov_char32_to_codepoint(ovutf_utf8_count, &n, src, src_len)) {
    return 0;
  }
  return n;
}

size_t ov_char32_to_utf8(
    char32_t const *const src, size_t const src_len, char *const dest, size_t const dest_len, size_t *const read) {
  if (!src || !src_len || !dest || !dest_len) {
    return 0;
  }
  struct ovutf_utf8_write_context ctx = {
      .cur = (uint8_t *)dest,
      .end = (uint8_t *)dest + dest_len,
  };
  size_t const r = ov_char32_to_codepoint(ovutf_utf8_write, &ctx, src, src_len);
  *ctx.cur = L'\0';
  if (!r) {
    return 0;
  }
  if (read) {
    *read = r;
  }
  return (size_t)(ctx.cur - (uint8_t *)dest);
}
