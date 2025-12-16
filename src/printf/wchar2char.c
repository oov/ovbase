#include <ovprintf.h>

#include "util.h"

#include <assert.h>

struct wchar2char_context {
  void (*putc)(int c, void *ctx);
  void *user_ctx;
  struct wchar_state state;
};

static void wchar2char_callback(int const c, void *const ctx) {
  struct wchar2char_context *const conv = (struct wchar2char_context *)ctx;
  int_fast32_t const cp = wchar_state_feed(&conv->state, (wchar_t)c);
  if (cp >= 0) {
    emit_codepoint_as_utf8(conv->putc, conv->user_ctx, cp);
  }
}

int ov_vpprintf_wchar2char(void (*const putc)(int c, void *ctx),
                           void *const ctx,
                           wchar_t const *const reference,
                           wchar_t const *const format,
                           va_list valist) {
  assert(putc != NULL && "putc must not be NULL");
  assert(format != NULL && "format must not be NULL");
  if (!putc || !format) {
    return -1;
  }

  struct wchar2char_context conv = {.putc = putc, .user_ctx = ctx};
  wchar_state_init(&conv.state);

  int const result = ov_vpprintf_wchar(wchar2char_callback, &conv, reference, format, valist);

  if (result >= 0 && !wchar_state_is_complete(&conv.state)) {
    return -1;
  }
  return result;
}

int ov_pprintf_wchar2char(void (*const putc)(int c, void *ctx),
                          void *const ctx,
                          wchar_t const *const reference,
                          wchar_t const *const format,
                          ...) {
  assert(putc != NULL && "putc must not be NULL");
  assert(format != NULL && "format must not be NULL");
  if (!putc || !format) {
    return -1;
  }

  va_list valist;
  va_start(valist, format);
  int const result = ov_vpprintf_wchar2char(putc, ctx, reference, format, valist);
  va_end(valist);
  return result;
}

struct snprintf_char_context {
  char *dest;
  char *end;
  size_t count;
};

static void snprintf_char_putc(int const c, void *const ctx) {
  struct snprintf_char_context *const sc = (struct snprintf_char_context *)ctx;
  sc->count++;
  if (sc->dest && sc->dest < sc->end) {
    *sc->dest++ = (char)c;
  }
}

struct vsnprintf_wchar2char_context {
  struct snprintf_char_context *snprintf_ctx;
  struct wchar_state state;
};

static void vsnprintf_wchar2char_callback(int const c, void *const ctx) {
  struct vsnprintf_wchar2char_context *const conv = (struct vsnprintf_wchar2char_context *)ctx;
  int_fast32_t const cp = wchar_state_feed(&conv->state, (wchar_t)c);
  if (cp >= 0) {
    emit_codepoint_as_utf8(snprintf_char_putc, conv->snprintf_ctx, cp);
  }
}

int ov_vsnprintf_wchar2char(char *const dest,
                            size_t const destlen,
                            wchar_t const *const reference,
                            wchar_t const *const format,
                            va_list valist) {
  assert(format != NULL && "format must not be NULL");
  if (!format) {
    return -1;
  }

  struct snprintf_char_context snprintf_ctx = {
      .dest = dest,
      .end = dest ? dest + destlen : NULL,
      .count = 0,
  };
  if (snprintf_ctx.end && snprintf_ctx.end > snprintf_ctx.dest) {
    snprintf_ctx.end--;
  }

  struct vsnprintf_wchar2char_context conv = {.snprintf_ctx = &snprintf_ctx};
  wchar_state_init(&conv.state);

  int const result = ov_vpprintf_wchar(vsnprintf_wchar2char_callback, &conv, reference, format, valist);

  if (dest && destlen > 0) {
    if (snprintf_ctx.dest <= snprintf_ctx.end) {
      *snprintf_ctx.dest = '\0';
    } else {
      dest[destlen - 1] = '\0';
    }
  }

  if (result >= 0 && !wchar_state_is_complete(&conv.state)) {
    return -1;
  }
  return (int)snprintf_ctx.count;
}

int ov_snprintf_wchar2char(
    char *const dest, size_t const destlen, wchar_t const *const reference, wchar_t const *const format, ...) {
  assert(format != NULL && "format must not be NULL");
  if (!format) {
    return -1;
  }

  va_list valist;
  va_start(valist, format);
  int const result = ov_vsnprintf_wchar2char(dest, destlen, reference, format, valist);
  va_end(valist);
  return result;
}
