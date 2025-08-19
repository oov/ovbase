#include <ovprintf_ex.h>

#include <ovarray.h>
#include <ovprintf.h>

struct put_char_context {
  wchar_t **const dest;
  size_t pos;
  error err;
};

static void put_char(int c, void *ctx) {
  struct put_char_context *pcctx = ctx;
  if (efailed(pcctx->err)) {
    return;
  }

  size_t const current_cap = *pcctx->dest ? OV_ARRAY_CAPACITY(*pcctx->dest) : 0;
  if (pcctx->pos >= current_cap) {
    size_t const new_cap = current_cap < 32 ? 64 : current_cap * 2;
    pcctx->err = OV_ARRAY_GROW(pcctx->dest, new_cap);
    if (efailed(pcctx->err)) {
      pcctx->err = ethru(pcctx->err);
      return;
    }
  }

  (*pcctx->dest)[pcctx->pos++] = (wchar_t)c;
}

NODISCARD error ov_vsprintf_wchar(wchar_t **const dest,
                                  wchar_t const *const reference,
                                  wchar_t const *const format,
                                  va_list valist) {
  if (!dest || !format) {
    return errg(err_invalid_arugment);
  }

  struct put_char_context ctx = {
      .dest = dest,
      .pos = 0,
      .err = eok(),
  };

  int const r = ov_vpprintf(put_char, &ctx, reference, format, valist);
  if (r == 0) {
    return errg(err_fail);
  }

  put_char(0, &ctx);
  if (efailed(ctx.err)) {
    ctx.err = ethru(ctx.err);
    return ctx.err;
  }

  OV_ARRAY_SET_LENGTH(*dest, (size_t)r);
  return eok();
}

NODISCARD error ov_sprintf_wchar(wchar_t **const dest,
                                 wchar_t const *const reference,
                                 wchar_t const *const format,
                                 ...) {
  va_list valist;
  va_start(valist, format);
  error err = ov_vsprintf_wchar(dest, reference, format, valist);
  va_end(valist);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  return eok();
}
