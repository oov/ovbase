#include "ovutf.h"

#include "common.h"

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
