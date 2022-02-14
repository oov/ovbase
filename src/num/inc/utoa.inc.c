#include "def.h"

CHAR_TYPE *FUNCNAME(utoa)(UINT_TYPE v, CHAR_TYPE buf[BUFFER_SIZE]) {
  //   8bits => "255\0"(4)
  //  16bits => "65535\0"(6)
  //  32bits => "4294967295\0"(11)
  //  64bits => "18446744073709551615\0"(21)
  // 128bits => "340282366920938463463374607431768211455\0"(40)
  // 256bits => "115792089237316195423570985008687907853269984665640564039457584007913129639935\0"(79)
  _Static_assert((sizeof(UINT_TYPE) == 1 && BUFFER_SIZE >= 4) || (sizeof(UINT_TYPE) == 2 && BUFFER_SIZE >= 6) ||
                     (sizeof(UINT_TYPE) == 4 && BUFFER_SIZE >= 11) || (sizeof(UINT_TYPE) == 8 && BUFFER_SIZE >= 21) ||
                     (sizeof(UINT_TYPE) == 16 && BUFFER_SIZE >= 40) || (sizeof(UINT_TYPE) == 32 && BUFFER_SIZE >= 79),
                 "Is it long enough to store the maximum value?");
  if (!buf) {
    return NULL;
  }
  CHAR_TYPE *c = buf + BUFFER_SIZE;
  UINT_TYPE v2;
  *--c = STR('\0');
  while (v >= 10) {
    v2 = v / 10;
    *--c = (CHAR_TYPE)(v - v2 * 10) + STR('0');
    v = v2;
  }
  *--c = (CHAR_TYPE)(v) + STR('0');
  return c;
}
