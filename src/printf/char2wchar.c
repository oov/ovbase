#include <ovprintf.h>

#include "util.h"

#include <assert.h>

struct char2wchar_context {
  void (*putc)(int c, void *ctx);
  void *user_ctx;
  struct utf8_state state;
};

static void char2wchar_callback(int const c, void *const ctx) {
  struct char2wchar_context *const conv = (struct char2wchar_context *)ctx;
  int_fast32_t const cp = utf8_state_feed(&conv->state, (uint8_t)c);
  if (cp >= 0) {
    emit_codepoint_as_wchar(conv->putc, conv->user_ctx, cp);
  }
}

int ov_vpprintf_char2wchar(void (*const putc)(int c, void *ctx),
                           void *const ctx,
                           char const *const reference,
                           char const *const format,
                           va_list valist) {
  assert(putc != NULL && "putc must not be NULL");
  assert(format != NULL && "format must not be NULL");
  if (!putc || !format) {
    return -1;
  }

  struct char2wchar_context conv = {.putc = putc, .user_ctx = ctx};
  utf8_state_init(&conv.state);

  int const result = ov_vpprintf_char(char2wchar_callback, &conv, reference, format, valist);

  if (result >= 0 && !utf8_state_is_complete(&conv.state)) {
    return -1;
  }
  return result;
}

int ov_pprintf_char2wchar(
    void (*const putc)(int c, void *ctx), void *const ctx, char const *const reference, char const *const format, ...) {
  assert(putc != NULL && "putc must not be NULL");
  assert(format != NULL && "format must not be NULL");
  if (!putc || !format) {
    return -1;
  }

  va_list valist;
  va_start(valist, format);
  int const result = ov_vpprintf_char2wchar(putc, ctx, reference, format, valist);
  va_end(valist);
  return result;
}

struct snprintf_wchar_context {
  wchar_t *dest;
  wchar_t *end;
  size_t count;
};

static void snprintf_wchar_putc(int const c, void *const ctx) {
  struct snprintf_wchar_context *const sc = (struct snprintf_wchar_context *)ctx;
  sc->count++;
  if (sc->dest && sc->dest < sc->end) {
    *sc->dest++ = (wchar_t)c;
  }
}

struct vsnprintf_char2wchar_context {
  struct snprintf_wchar_context *snprintf_ctx;
  struct utf8_state state;
};

static void vsnprintf_char2wchar_callback(int const c, void *const ctx) {
  struct vsnprintf_char2wchar_context *const conv = (struct vsnprintf_char2wchar_context *)ctx;
  int_fast32_t const cp = utf8_state_feed(&conv->state, (uint8_t)c);
  if (cp >= 0) {
    emit_codepoint_as_wchar(snprintf_wchar_putc, conv->snprintf_ctx, cp);
  }
}

int ov_vsnprintf_char2wchar(
    wchar_t *const dest, size_t const destlen, char const *const reference, char const *const format, va_list valist) {
  assert(format != NULL && "format must not be NULL");
  if (!format) {
    return -1;
  }

  struct snprintf_wchar_context snprintf_ctx = {
      .dest = dest,
      .end = dest ? dest + destlen : NULL,
      .count = 0,
  };
  if (snprintf_ctx.end && snprintf_ctx.end > snprintf_ctx.dest) {
    snprintf_ctx.end--;
  }

  struct vsnprintf_char2wchar_context conv = {.snprintf_ctx = &snprintf_ctx};
  utf8_state_init(&conv.state);

  int const result = ov_vpprintf_char(vsnprintf_char2wchar_callback, &conv, reference, format, valist);

  if (dest && destlen > 0) {
    if (snprintf_ctx.dest <= snprintf_ctx.end) {
      *snprintf_ctx.dest = L'\0';
    } else {
      dest[destlen - 1] = L'\0';
    }
  }

  if (result >= 0 && !utf8_state_is_complete(&conv.state)) {
    return -1;
  }
  return (int)snprintf_ctx.count;
}

int ov_snprintf_char2wchar(
    wchar_t *const dest, size_t const destlen, char const *const reference, char const *const format, ...) {
  assert(format != NULL && "format must not be NULL");
  if (!format) {
    return -1;
  }

  va_list valist;
  va_start(valist, format);
  int const result = ov_vsnprintf_char2wchar(dest, destlen, reference, format, valist);
  va_end(valist);
  return result;
}
