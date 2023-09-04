#include <ovutf.h>

#include "common.h"

static inline size_t first_byte_to_len(int const ch) {
  if (ch <= 0x7f) {
    return 1;
  }
  if ((ch & 0xe0) == 0xc0) {
    return 2;
  }
  if ((ch & 0xf0) == 0xe0) {
    return 3;
  }
  if ((ch & 0xf8) == 0xf0) {
    return 4;
  }
  return 0;
}

static inline bool u8later(int const ch) { return (ch & 0xc0) == 0x80; }

size_t ov_utf8_to_codepoint(ov_codepoint_fn fn, void *ctx, char const *const src, size_t const src_len) {
  if (!fn || !src || !src_len) {
    return 0;
  }
  uint8_t const *const u8 = (uint8_t const *)src;
  size_t i = 0;
  while (i < src_len) {
    size_t const ch_len = first_byte_to_len(u8[i]);
    if (!ch_len || i + ch_len > src_len) {
      return 0;
    }
    int_fast32_t codepoint = 0;
    switch (ch_len) {
    case 1:
      codepoint = u8[i];
      break;
    case 2:
      if (!u8later(u8[i + 1])) {
        return 0;
      }
      if (!(u8[i] & 0x1e)) {
        return 0;
      }
      codepoint = ((int_fast32_t)(u8[i] & 0x1f) << 6) | (u8[i + 1] & 0x3f);
      break;
    case 3:
      if (!u8later(u8[i + 1]) || !u8later(u8[i + 2])) {
        return 0;
      }
      if ((u8[i] == 0xe0) && !(u8[i + 1] & 0x20)) {
        return 0;
      }
      codepoint = ((int_fast32_t)(u8[i] & 0x0f) << 12) | ((int_fast32_t)(u8[i + 1] & 0x3f) << 6) |
                  (int_fast32_t)(u8[i + 2] & 0x3f);
      break;
    case 4:
      if (!u8later(u8[i + 1]) || !u8later(u8[i + 2]) || !u8later(u8[i + 3])) {
        return 0;
      }
      if ((u8[i] & 0x07) + (u8[i + 1] & 0x30) == 0) {
        return 0;
      }
      codepoint = ((int_fast32_t)(u8[i] & 0x07) << 18) | ((int_fast32_t)(u8[i + 1] & 0x3f) << 12) |
                  ((int_fast32_t)(u8[i + 2] & 0x3f) << 6) | (int_fast32_t)(u8[i + 3] & 0x3f);
      break;
    }
    if (invalid_codepoint(codepoint)) {
      return 0;
    }
    enum ov_codepoint_fn_result r = fn(codepoint, ctx);
    if (r != ov_codepoint_fn_result_abort) {
      i += ch_len;
    }
    if (r != ov_codepoint_fn_result_continue) {
      break;
    }
  }
  return i;
}

static enum ov_codepoint_fn_result count(int_fast32_t codepoint, void *ctx) {
  size_t *n = ctx;
  *n += sizeof(wchar_t) == 2 && codepoint > 0xffff ? 2 : 1;
  return ov_codepoint_fn_result_continue;
}

size_t ov_utf8_to_wchar_len(char const *const src, size_t const src_len) {
  size_t n = 0;
  if (!ov_utf8_to_codepoint(count, &n, src, src_len)) {
    return 0;
  }
  return n;
}

struct context {
  wchar_t *cur;
  wchar_t *end;
};

static enum ov_codepoint_fn_result write(int_fast32_t codepoint, void *ctx) {
  struct context *c = ctx;
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
  struct context ctx = {
      .cur = dest,
      .end = dest + dest_len,
  };
  size_t const r = ov_utf8_to_codepoint(write, &ctx, src, src_len);
  *ctx.cur = L'\0';
  if (!r) {
    return 0;
  }
  if (read) {
    *read = r;
  }
  return (size_t)(ctx.cur - dest);
}
