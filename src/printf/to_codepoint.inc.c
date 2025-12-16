#include <ovprintf.h>

#include "util.h"

#include <assert.h>

struct to_codepoint_context {
  void (*putc)(int_fast32_t c, void *ctx);
  void *user_ctx;
  STATE_TYPE state;
};

static void to_codepoint_callback(int const c, void *const ctx) {
  struct to_codepoint_context *const conv = (struct to_codepoint_context *)ctx;
  int_fast32_t const cp = STATE_FEED(&conv->state, STATE_FEED_CAST c);
  if (cp >= 0) {
    emit_codepoint_as_codepoint(conv->putc, conv->user_ctx, cp);
  }
}

int FUNCNAME_VPPRINTF(void (*const putc)(int_fast32_t c, void *ctx),
                      void *const ctx,
                      SRC_CHAR_TYPE const *const reference,
                      SRC_CHAR_TYPE const *const format,
                      va_list valist) {
  assert(putc != NULL && "putc must not be NULL");
  assert(format != NULL && "format must not be NULL");
  if (!putc || !format) {
    return -1;
  }

  struct to_codepoint_context conv = {.putc = putc, .user_ctx = ctx};
  STATE_INIT(&conv.state);

  int const result = VPPRINTF_SRC(to_codepoint_callback, &conv, reference, format, valist);

  if (result >= 0 && !STATE_IS_COMPLETE(&conv.state)) {
    return -1;
  }
  return result;
}

int FUNCNAME_PPRINTF(void (*const putc)(int_fast32_t c, void *ctx),
                     void *const ctx,
                     SRC_CHAR_TYPE const *const reference,
                     SRC_CHAR_TYPE const *const format,
                     ...) {
  assert(putc != NULL && "putc must not be NULL");
  assert(format != NULL && "format must not be NULL");
  if (!putc || !format) {
    return -1;
  }

  va_list valist;
  va_start(valist, format);
  int const result = FUNCNAME_VPPRINTF(putc, ctx, reference, format, valist);
  va_end(valist);
  return result;
}
