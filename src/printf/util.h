#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

enum {
  codepoint_incomplete = -1,
  codepoint_error = -2,
};

static inline bool utf8_is_continuation(uint8_t const ch) { return (ch & 0xc0) == 0x80; }

static inline size_t utf8_first_byte_to_len(uint8_t const ch) {
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

static inline bool codepoint_is_invalid(int_fast32_t const ch) {
  return ch < 0 || ch > 0x10ffff || (0xd800 <= ch && ch < 0xe000);
}

static inline bool wchar_is_surrogate_high(wchar_t const ch) { return 0xd800 <= ch && ch < 0xdc00; }

static inline bool wchar_is_surrogate_low(wchar_t const ch) { return 0xdc00 <= ch && ch < 0xe000; }

static inline int_fast32_t utf8_buf_to_codepoint(uint8_t const *const buf, size_t const len) {
  int_fast32_t codepoint = 0;
  switch (len) {
  case 1:
    codepoint = buf[0];
    break;
  case 2:
    if (!utf8_is_continuation(buf[1])) {
      return -1;
    }
    if (!(buf[0] & 0x1e)) {
      return -1;
    }
    codepoint = ((int_fast32_t)(buf[0] & 0x1f) << 6) | (buf[1] & 0x3f);
    break;
  case 3:
    if (!utf8_is_continuation(buf[1]) || !utf8_is_continuation(buf[2])) {
      return -1;
    }
    if ((buf[0] == 0xe0) && !(buf[1] & 0x20)) {
      return -1;
    }
    codepoint = ((int_fast32_t)(buf[0] & 0x0f) << 12) | ((int_fast32_t)(buf[1] & 0x3f) << 6) | (buf[2] & 0x3f);
    break;
  case 4:
    if (!utf8_is_continuation(buf[1]) || !utf8_is_continuation(buf[2]) || !utf8_is_continuation(buf[3])) {
      return -1;
    }
    if ((buf[0] & 0x07) + (buf[1] & 0x30) == 0) {
      return -1;
    }
    codepoint = ((int_fast32_t)(buf[0] & 0x07) << 18) | ((int_fast32_t)(buf[1] & 0x3f) << 12) |
                ((int_fast32_t)(buf[2] & 0x3f) << 6) | (buf[3] & 0x3f);
    break;
  default:
    return -1;
  }
  if (codepoint_is_invalid(codepoint)) {
    return -1;
  }
  return codepoint;
}

struct utf8_state {
  uint8_t buf[4];
  uint8_t buf_len;
  uint8_t expected_len;
};

static inline void utf8_state_init(struct utf8_state *const s) {
  s->buf_len = 0;
  s->expected_len = 0;
}

static inline bool utf8_state_is_complete(struct utf8_state const *const s) { return s->buf_len == 0; }

static inline int_fast32_t utf8_state_feed(struct utf8_state *const s, uint8_t const byte) {
  if (s->buf_len == 0) {
    s->expected_len = (uint8_t)utf8_first_byte_to_len(byte);
    if (s->expected_len == 0) {
      return codepoint_error;
    }
    s->buf[0] = byte;
    s->buf_len = 1;
  } else {
    if (!utf8_is_continuation(byte)) {
      s->buf_len = 0;
      return codepoint_error;
    }
    s->buf[s->buf_len++] = byte;
  }

  if (s->buf_len == s->expected_len) {
    int_fast32_t const codepoint = utf8_buf_to_codepoint(s->buf, s->buf_len);
    s->buf_len = 0;
    s->expected_len = 0;
    return (codepoint >= 0) ? codepoint : codepoint_error;
  }

  return codepoint_incomplete;
}

struct wchar_state {
  wchar_t high_surrogate;
  bool has_high;
};

static inline void wchar_state_init(struct wchar_state *const s) {
  s->high_surrogate = 0;
  s->has_high = false;
}

static inline bool wchar_state_is_complete(struct wchar_state const *const s) { return !s->has_high; }

static inline int_fast32_t wchar_state_feed(struct wchar_state *const s, wchar_t const wc) {
  if (sizeof(wchar_t) == 2) {
    if (!s->has_high) {
      if (wchar_is_surrogate_high(wc)) {
        s->high_surrogate = wc;
        s->has_high = true;
        return codepoint_incomplete;
      }
      if (wchar_is_surrogate_low(wc)) {
        return codepoint_error;
      }
      int_fast32_t const codepoint = (int_fast32_t)wc;
      return codepoint_is_invalid(codepoint) ? codepoint_error : codepoint;
    } else {
      s->has_high = false;
      if (!wchar_is_surrogate_low(wc)) {
        return codepoint_error;
      }
      int_fast32_t const codepoint =
          0x10000 + ((int_fast32_t)(s->high_surrogate) - 0xd800) * 0x400 + ((int_fast32_t)(wc)-0xdc00);
      return codepoint_is_invalid(codepoint) ? codepoint_error : codepoint;
    }
  } else {
    int_fast32_t const codepoint = (int_fast32_t)wc;
    return codepoint_is_invalid(codepoint) ? codepoint_error : codepoint;
  }
}

static inline void
emit_codepoint_as_utf8(void (*const putc)(int c, void *ctx), void *const user_ctx, int_fast32_t const codepoint) {
  if (codepoint < 0x80) {
    putc((int)(codepoint & 0x7f), user_ctx);
  } else if (codepoint < 0x800) {
    putc((int)(0xc0 | ((codepoint >> 6) & 0x1f)), user_ctx);
    putc((int)(0x80 | (codepoint & 0x3f)), user_ctx);
  } else if (codepoint < 0x10000) {
    putc((int)(0xe0 | ((codepoint >> 12) & 0x0f)), user_ctx);
    putc((int)(0x80 | ((codepoint >> 6) & 0x3f)), user_ctx);
    putc((int)(0x80 | (codepoint & 0x3f)), user_ctx);
  } else {
    putc((int)(0xf0 | ((codepoint >> 18) & 0x07)), user_ctx);
    putc((int)(0x80 | ((codepoint >> 12) & 0x3f)), user_ctx);
    putc((int)(0x80 | ((codepoint >> 6) & 0x3f)), user_ctx);
    putc((int)(0x80 | (codepoint & 0x3f)), user_ctx);
  }
}

static inline void
emit_codepoint_as_wchar(void (*const putc)(int c, void *ctx), void *const user_ctx, int_fast32_t const codepoint) {
  if (sizeof(wchar_t) == 2 && codepoint > 0xffff) {
    int_fast32_t const adjusted = codepoint - 0x10000;
    wchar_t const high = (wchar_t)(0xd800 + (adjusted >> 10));
    wchar_t const low = (wchar_t)(0xdc00 + (adjusted & 0x3ff));
    putc((int)high, user_ctx);
    putc((int)low, user_ctx);
  } else {
    putc((int)(wchar_t)codepoint, user_ctx);
  }
}

static inline void emit_codepoint_as_codepoint(void (*const putc)(int_fast32_t c, void *ctx),
                                               void *const user_ctx,
                                               int_fast32_t const codepoint) {
  putc(codepoint, user_ctx);
}
