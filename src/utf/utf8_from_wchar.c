#include <ovutf.h>

#include "common.h"

#include <assert.h>

size_t ov_wchar_to_utf8_len(wchar_t const *const src, size_t const src_len) {
  assert(src != NULL || src_len == 0 && "src must not be NULL when src_len > 0");
  if (!src || !src_len) {
    return 0;
  }
  size_t n = 0;
  if (!ov_wchar_to_codepoint(ovutf_utf8_count, &n, src, src_len)) {
    return 0;
  }
  return n;
}

size_t ov_wchar_to_utf8(
    wchar_t const *const src, size_t const src_len, char *const dest, size_t const dest_len, size_t *const read) {
  assert(src != NULL || src_len == 0 && "src must not be NULL when src_len > 0");
  assert(dest != NULL || dest_len == 0 && "dest must not be NULL when dest_len > 0");
  if (!src || !src_len || !dest || !dest_len) {
    return 0;
  }
  struct ovutf_utf8_write_context ctx = {
      .cur = (uint8_t *)dest,
      .end = (uint8_t *)dest + dest_len,
  };
  size_t const r = ov_wchar_to_codepoint(ovutf_utf8_write, &ctx, src, src_len);
  *ctx.cur = '\0';
  if (!r) {
    return 0;
  }
  if (read) {
    *read = r;
  }
  return (size_t)(ctx.cur - (uint8_t *)dest);
}
