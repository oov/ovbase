#include "codepoint_utf8.h"

#include "common.h"

static enum ov_codepoint_fn_result count_wchar(int_fast32_t codepoint, void *ctx) {
  size_t *n = (size_t *)ctx;
  *n += sizeof(wchar_t) == 2 && codepoint > 0xffff ? 2 : 1;
  return ov_codepoint_fn_result_continue;
}

size_t ov_utf8_to_wchar_len(char const *const src, size_t const src_len) {
  size_t n = 0;
  if (!ov_utf8_to_codepoint(count_wchar, &n, src, src_len)) {
    return 0;
  }
  return n;
}

struct wchar_context {
  wchar_t *cur;
  wchar_t *end;
};

static enum ov_codepoint_fn_result write_wchar(int_fast32_t codepoint, void *ctx) {
  struct wchar_context *c = (struct wchar_context *)ctx;
  if (sizeof(wchar_t) == 2 && codepoint > 0xffff) {
    if (c->end - c->cur <= 2) {
      return ov_codepoint_fn_result_abort;
    }
    *c->cur++ = (wchar_t)((codepoint - 0x10000) / 0x400 + 0xd800);
    *c->cur++ = (wchar_t)((codepoint - 0x10000) % 0x400 + 0xdc00);
    return ov_codepoint_fn_result_continue;
  }
  if (c->end - c->cur <= 1) {
    return ov_codepoint_fn_result_abort;
  }
  *c->cur++ = (wchar_t)codepoint;
  return ov_codepoint_fn_result_continue;
}

size_t ov_utf8_to_wchar(
    char const *const src, size_t const src_len, wchar_t *const dest, size_t const dest_len, size_t *const read) {
  if (!src || !src_len || !dest || !dest_len) {
    return 0;
  }
  struct wchar_context ctx = {
      .cur = dest,
      .end = dest + dest_len,
  };
  size_t const r = ov_utf8_to_codepoint(write_wchar, &ctx, src, src_len);
  *ctx.cur = L'\0';
  if (!r) {
    return 0;
  }
  if (read) {
    *read = r;
  }
  return (size_t)(ctx.cur - dest);
}
