#include "ovutf.h"

#include "common.h"

static inline bool surrogate_high(int const ch) { return 0xd800 <= ch && ch < 0xdc00; }

static inline bool surrogate_low(int const ch) { return 0xdc00 <= ch && ch < 0xe000; }

size_t ov_wchar_to_codepoint(ov_codepoint_fn fn, void *ctx, wchar_t const *const src, size_t const src_len) {
  size_t i = 0;
  while (i < src_len) {
    int_fast32_t codepoint = 0;
    if (sizeof(wchar_t) == 2 && surrogate_high(src[i])) {
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
      i += (sizeof(wchar_t) == 2 && codepoint > 0xffff) ? 2 : 1;
    }
    if (r != ov_codepoint_fn_result_continue) {
      break;
    }
  }
  return i;
}

static inline size_t codepoint_to_utf8len(int_fast32_t codepoint) {
  if (codepoint < 0x80) {
    return 1;
  } else if (codepoint < 0x800) {
    return 2;
  } else if (codepoint < 0x10000) {
    return 3;
  }
  return 4;
}

static enum ov_codepoint_fn_result count(int_fast32_t codepoint, void *ctx) {
  size_t *n = ctx;
  *n += codepoint_to_utf8len(codepoint);
  return ov_codepoint_fn_result_continue;
}

size_t ov_wchar_to_utf8_len(wchar_t const *const src, size_t const src_len) {
  size_t n = 0;
  if (!ov_wchar_to_codepoint(count, &n, src, src_len)) {
    return 0;
  }
  return n;
}

struct context {
  uint8_t *cur;
  uint8_t *end;
};

static enum ov_codepoint_fn_result write(int_fast32_t codepoint, void *ctx) {
  struct context *c = ctx;
  size_t const n = codepoint_to_utf8len(codepoint);
  if ((size_t)(c->end - c->cur) <= n) {
    return ov_codepoint_fn_result_abort;
  }
  switch (n) {
  case 1:
    *c->cur++ = codepoint & 0x7f;
    break;
  case 2:
    *c->cur++ = 0xc0 | ((codepoint >> 6) & 0x1f);
    *c->cur++ = 0x80 | (codepoint & 0x3f);
    break;
  case 3:
    *c->cur++ = 0xe0 | ((codepoint >> 12) & 0x0f);
    *c->cur++ = 0x80 | ((codepoint >> 6) & 0x3f);
    *c->cur++ = 0x80 | (codepoint & 0x3f);
    break;
  case 4:
    *c->cur++ = 0xf0 | ((codepoint >> 18) & 0x07);
    *c->cur++ = 0x80 | ((codepoint >> 12) & 0x3f);
    *c->cur++ = 0x80 | ((codepoint >> 6) & 0x3f);
    *c->cur++ = 0x80 | (codepoint & 0x3f);
    break;
  }
  return ov_codepoint_fn_result_continue;
}

size_t ov_wchar_to_utf8(
    wchar_t const *const src, size_t const src_len, char *const dest, size_t const dest_len, size_t *const read) {
  if (!src || !src_len || !dest || !dest_len) {
    return 0;
  }
  struct context ctx = {
      .cur = (uint8_t *)dest,
      .end = (uint8_t *)dest + dest_len,
  };
  size_t const r = ov_wchar_to_codepoint(write, &ctx, src, src_len);
  *ctx.cur = L'\0';
  if (!r) {
    return 0;
  }
  if (read) {
    *read = r;
  }
  return (size_t)(ctx.cur - (uint8_t *)dest);
}
