#include "wstr.h"

#ifdef USE_WSTR

#  include "ovprintf.h"

NODISCARD error wstr_sprintf_(struct wstr *const dest MEM_FILEPOS_PARAMS,
                              wchar_t const *const reference,
                              wchar_t const *const format,
                              ...) {
  va_list valist;
  va_start(valist, format);
  error err = wstr_vsprintf_(dest MEM_FILEPOS_VALUES_PASSTHRU, reference, format, valist);
  if (efailed(err)) {
    err = ethru(err);
  }
  va_end(valist);
  return err;
}

struct put_char_context {
  struct wstr *const str;
  size_t pos;
  error err;
#  ifdef ALLOCATE_LOGGER
  struct ov_filepos const *const filepos;
#  endif
};

static void put_char(int c, void *ctx) {
  struct put_char_context *pcctx = ctx;
  if (efailed(pcctx->err)) {
    return;
  }
  size_t const cap = pcctx->str->cap;
  if (pcctx->pos >= cap) {
#  ifdef ALLOCATE_LOGGER
    struct ov_filepos const *const filepos = pcctx->filepos;
#  endif
    pcctx->err = wstr_grow_(pcctx->str, cap < 32 ? 64 : cap * 2 MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(pcctx->err)) {
      pcctx->err = ethru(pcctx->err);
      return;
    }
  }
  pcctx->str->ptr[pcctx->pos++] = (wchar_t)c;
}

NODISCARD error wstr_vsprintf_(struct wstr *const dest MEM_FILEPOS_PARAMS,
                               wchar_t const *const reference,
                               wchar_t const *const format,
                               va_list valist) {
  if (!dest || !format) {
    return errg(err_invalid_arugment);
  }
  struct put_char_context ctx = {
      .str = dest,
      .err = eok(),
#  ifdef ALLOCATE_LOGGER
      .filepos = filepos,
#  endif

  };
  int const r = ov_vpprintf(put_char, &ctx, reference, format, valist);
  if (r == 0) {
    return errg(err_fail); // invalid format?
  }
  put_char(0, &ctx);
  if (efailed(ctx.err)) {
    ctx.err = ethru(ctx.err);
    return ctx.err;
  }
  dest->len = (size_t)r;
  return eok();
}

#endif
