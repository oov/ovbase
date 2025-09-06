#include "common.h"

#include <ovutf.h>

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

enum ov_codepoint_fn_result ovutf_utf8_count(int_fast32_t codepoint, void *ctx) {
  size_t *n = (size_t *)ctx;
  *n += codepoint_to_utf8len(codepoint);
  return ov_codepoint_fn_result_continue;
}

enum ov_codepoint_fn_result ovutf_utf8_write(int_fast32_t const codepoint, void *ctx) {
  struct ovutf_utf8_write_context *const c = (struct ovutf_utf8_write_context *)ctx;
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
