#include <ovutf.h>

#include <assert.h>

static enum ov_codepoint_fn_result count_char16(int_fast32_t codepoint, void *ctx) {
  assert(ctx != NULL && "ctx must not be NULL");
  size_t *n = (size_t *)ctx;
  *n += codepoint > 0xffff ? 2 : 1;
  return ov_codepoint_fn_result_continue;
}

size_t ov_utf8_to_char16_len(char const *const src, size_t const src_len) {
  assert(src != NULL || src_len == 0 && "src must not be NULL when src_len > 0");
  if (!src || !src_len) {
    return 0;
  }
  size_t n = 0;
  if (!ov_utf8_to_codepoint(count_char16, &n, src, src_len)) {
    return 0;
  }
  return n;
}

struct char16_context {
  char16_t *cur;
  char16_t *end;
};

static enum ov_codepoint_fn_result write_char16(int_fast32_t codepoint, void *ctx) {
  assert(ctx != NULL && "ctx must not be NULL");
  struct char16_context *c = (struct char16_context *)ctx;
  assert(c->cur != NULL && "c->cur must not be NULL");
  assert(c->end != NULL && "c->end must not be NULL");
  if (codepoint > 0xffff) {
    if (c->end - c->cur <= 2) {
      return ov_codepoint_fn_result_abort;
    }
    *c->cur++ = (char16_t)((codepoint - 0x10000) / 0x400 + 0xd800);
    *c->cur++ = (char16_t)((codepoint - 0x10000) % 0x400 + 0xdc00);
    return ov_codepoint_fn_result_continue;
  }
  if (c->end - c->cur <= 1) {
    return ov_codepoint_fn_result_abort;
  }
  *c->cur++ = (char16_t)codepoint;
  return ov_codepoint_fn_result_continue;
}

size_t ov_utf8_to_char16(
    char const *const src, size_t const src_len, char16_t *const dest, size_t const dest_len, size_t *const read) {
  assert(src != NULL || src_len == 0 && "src must not be NULL when src_len > 0");
  assert(dest != NULL || dest_len == 0 && "dest must not be NULL when dest_len > 0");
  if (!src || !src_len || !dest || !dest_len) {
    return 0;
  }
  struct char16_context ctx = {
      .cur = dest,
      .end = dest + dest_len,
  };
  size_t const r = ov_utf8_to_codepoint(write_char16, &ctx, src, src_len);
  *ctx.cur = L'\0';
  if (!r) {
    return 0;
  }
  if (read) {
    *read = r;
  }
  return (size_t)(ctx.cur - dest);
}
