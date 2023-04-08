#pragma once

#include <stdbool.h>
#include <stdint.h>

static inline bool invalid_codepoint(int_fast32_t ch) {
  return ch < 0 || ch > 0x10ffff || (0xd800 <= ch && ch < 0xe000);
}
