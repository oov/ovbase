#include "def.h"

#include <assert.h>
#include <math.h>

CHAR_TYPE *
FUNCNAME(ftoa)(FLOAT_TYPE const d, size_t const frac_len, CHAR_TYPE const dot, CHAR_TYPE buf[BUFFER_SIZE_FLOAT]) {
  _Static_assert((sizeof(INT_TYPE) == 1 && BUFFER_SIZE_FLOAT >= 5 * 2) ||
                     (sizeof(INT_TYPE) == 2 && BUFFER_SIZE_FLOAT >= 7 * 2) ||
                     (sizeof(INT_TYPE) == 4 && BUFFER_SIZE_FLOAT >= 12 * 2) ||
                     (sizeof(INT_TYPE) == 8 && BUFFER_SIZE_FLOAT >= 21 * 2) ||
                     (sizeof(INT_TYPE) == 16 && BUFFER_SIZE_FLOAT >= 41 * 2) ||
                     (sizeof(INT_TYPE) == 32 && BUFFER_SIZE_FLOAT >= 79 * 2),
                 "Is it long enough to store the value?");
  assert(buf != NULL && "buf must not be NULL");
  assert(frac_len <= BUFFER_SIZE_FLOAT / 2 && "frac_len must not exceed buffer capacity");
  if (!buf || frac_len > BUFFER_SIZE_FLOAT / 2) {
    return NULL;
  }
  INT_TYPE const iv = (INT_TYPE)d;
  UINT_TYPE uv, uv2;
  CHAR_TYPE *c = buf + BUFFER_SIZE_FLOAT;
  *--c = STR('\0');
  if (frac_len) {
    uv = (UINT_TYPE)(round(fabs(d - (FLOAT_TYPE)iv) * pow(10, (FLOAT_TYPE)frac_len)));
    for (size_t i = 0; i < frac_len; ++i) {
      uv2 = uv / 10;
      *--c = (CHAR_TYPE)(uv - uv2 * 10) + STR('0');
      uv = uv2;
    }
    *--c = dot;
    uv = (UINT_TYPE)(iv < 0 ? -iv : iv);
  } else {
    uv = (UINT_TYPE)(round(fabs(d)));
  }
  while (uv >= 10) {
    uv2 = uv / 10;
    *--c = (CHAR_TYPE)(uv - uv2 * 10) + STR('0');
    uv = uv2;
  }
  *--c = (CHAR_TYPE)(uv) + STR('0');
  if (d < 0) {
    *--c = STR('-');
  }
  return c;
}
