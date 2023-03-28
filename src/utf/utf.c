#include "ovutf.h"

#include <stdint.h>

static inline size_t u8charlen(int const ch) {
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

static inline bool invalid_codepoint(int32_t ch) { return ch < 0 || ch > 0x10ffff || (0xd800 <= ch && ch < 0xe000); }

bool utf8_to_wchar(char const *const src,
                   size_t const src_len,
                   wchar_t *const dest,
                   size_t const dest_len,
                   size_t *const read,
                   size_t *const written) {
  uint8_t const *const u8 = (uint8_t const *)src;
  size_t i = 0, j = 0;
  while (i < src_len) {
    size_t const ch_len = u8charlen(u8[i]);
    if (!ch_len || i + ch_len > src_len) {
      return false;
    }
    int32_t codepoint = 0;
    switch (ch_len) {
    case 1:
      codepoint = u8[i];
      break;
    case 2:
      if (!u8later(u8[i + 1])) {
        return false;
      }
      if (!(u8[i + 1] & 0x1e)) {
        return false;
      }
      codepoint = ((int32_t)(u8[i] & 0x1f) << 6) | (u8[i + 1] & 0x3f);
      break;
    case 3:
      if (!u8later(u8[i + 1]) || !u8later(u8[i + 2])) {
        return false;
      }
      if ((u8[i] == 0xe0) && !(u8[i + 1] & 0x20)) {
        return false;
      }
      codepoint = ((int32_t)(u8[i] & 0x0f) << 12) | ((int32_t)(u8[i + 1] & 0x3f) << 6) | (int32_t)(u8[i + 2] & 0x3f);
      break;
    case 4:
      if (!u8later(u8[i + 1]) || !u8later(u8[i + 2]) || !u8later(u8[i + 3])) {
        return false;
      }
      if ((u8[i] & 0x07) + (u8[i + 1] & 0x30) == 0) {
        return false;
      }
      codepoint = ((int32_t)(u8[i] & 0x07) << 18) | ((int32_t)(u8[i + 1] & 0x3f) << 12) |
                  ((int32_t)(u8[i + 2] & 0x3f) << 6) | (int32_t)(u8[i + 3] & 0x3f);
      break;
    }
    if (invalid_codepoint(codepoint)) {
      return false;
    }
    if (sizeof(wchar_t) == 2 && codepoint > 0xffff) {
      if (dest) {
        if (j + 2 > dest_len) {
          break;
        }
        dest[j] = (wchar_t)((codepoint - 0x10000) / 0x400 + 0xd800);
        dest[j + 1] = (wchar_t)((codepoint - 0x10000) % 0x400 + 0xdc00);
      }
      j += 2;
    } else {
      if (dest) {
        if (j + 1 > dest_len) {
          break;
        }
        dest[j] = (wchar_t)codepoint;
      }
      ++j;
    }
    i += ch_len;
  }
  if (read) {
    *read = i;
  }
  if (written) {
    *written = j;
  }
  return true;
}

bool utf8_to_wchar_len(char const *const src, size_t const src_len, size_t *const len) {
  return utf8_to_wchar(src, src_len, NULL, 0, NULL, len);
}

static inline bool surrogate_high(int const ch) { return 0xd800 <= ch && ch < 0xdc00; }

static inline bool surrogate_low(int const ch) { return 0xdc00 <= ch && ch < 0xe000; }

bool wchar_to_utf8(wchar_t const *const src,
                   size_t const src_len,
                   char *const dest,
                   size_t const dest_len,
                   size_t *const read,
                   size_t *const written) {
  uint8_t *const u8 = (uint8_t *)dest;
  size_t i = 0, j = 0;
  while (i < src_len) {
    int32_t codepoint = 0;
    if (sizeof(wchar_t) == 2 && surrogate_high(src[i])) {
      if (i + 2 > src_len || !surrogate_low(src[i + 1])) {
        return false;
      }
      codepoint = 0x10000 + ((int32_t)(src[i]) - 0xd800) * 0x400 + ((int32_t)(src[i + 1]) - 0xdc00);
    } else {
      codepoint = (int32_t)(src[i]);
    }
    if (invalid_codepoint(codepoint)) {
      return false;
    }
    if (codepoint < 0x80) {
      if (dest) {
        if (j + 1 > dest_len) {
          break;
        }
        u8[j] = codepoint & 0x7f;
      }
      ++j;
    } else if (codepoint < 0x800) {
      if (dest) {
        if (j + 2 > dest_len) {
          break;
        }
        u8[j] = 0xc0 | ((codepoint >> 6) & 0x1f);
        u8[j + 1] = 0x80 | (codepoint & 0x3f);
      }
      j += 2;
    } else if (codepoint < 0x10000) {
      if (dest) {
        if (j + 3 > dest_len) {
          break;
        }
        u8[j] = 0xe0 | ((codepoint >> 12) & 0x0f);
        u8[j + 1] = 0x80 | ((codepoint >> 6) & 0x3f);
        u8[j + 2] = 0x80 | (codepoint & 0x3f);
      }
      j += 3;
    } else {
      if (dest) {
        if (j + 4 > dest_len) {
          break;
        }
        u8[j] = 0xf0 | ((codepoint >> 18) & 0x07);
        u8[j + 1] = 0x80 | ((codepoint >> 12) & 0x3f);
        u8[j + 2] = 0x80 | ((codepoint >> 6) & 0x3f);
        u8[j + 3] = 0x80 | (codepoint & 0x3f);
      }
      j += 4;
    }
    if (sizeof(wchar_t) == 2 && codepoint > 0xffff) {
      i += 2;
    } else {
      ++i;
    }
  }
  if (read) {
    *read = i;
  }
  if (written) {
    *written = j;
  }
  return true;
}

bool wchar_to_utf8_len(wchar_t const *const src, size_t const src_len, size_t *const len) {
  return wchar_to_utf8(src, src_len, NULL, 0, NULL, len);
}
