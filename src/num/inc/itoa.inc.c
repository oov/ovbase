#include "def.h"

CHAR_TYPE *FUNCNAME(itoa)(INT_TYPE v, CHAR_TYPE buf[BUFFER_SIZE]) {
  //   8bits => "-128\0"(5)
  //  16bits => "-32768\0"(7)
  //  32bits => "-2147483648\0"(12)
  //  64bits => "-9223372036854775808\0"(21)
  // 128bits => "-170141183460469231731687303715884105728\0"(41)
  // 256bits => "-57896044618658097711785492504343953926634992332820282019728792003956564819968\0"(79)
  _Static_assert((sizeof(INT_TYPE) == 1 && BUFFER_SIZE >= 5) || (sizeof(INT_TYPE) == 2 && BUFFER_SIZE >= 7) ||
                     (sizeof(INT_TYPE) == 4 && BUFFER_SIZE >= 12) || (sizeof(INT_TYPE) == 8 && BUFFER_SIZE >= 21) ||
                     (sizeof(INT_TYPE) == 16 && BUFFER_SIZE >= 41) || (sizeof(INT_TYPE) == 32 && BUFFER_SIZE >= 79),
                 "Is it long enough to store the minimum value?");
  if (!buf) {
    return NULL;
  }
  CHAR_TYPE *c = buf + BUFFER_SIZE;
  UINT_TYPE uv = (UINT_TYPE)(v < 0 ? -v : v), uv2;
  *--c = STR('\0');
  while (uv >= 10) {
    uv2 = uv / 10;
    *--c = (CHAR_TYPE)(uv - uv2 * 10) + STR('0');
    uv = uv2;
  }
  *--c = (CHAR_TYPE)(uv) + STR('0');
  if (v < 0) {
    *--c = STR('-');
  }
  return c;
}
