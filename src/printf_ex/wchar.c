#include <ovprintf_ex.h>

#include <ovarray.h>
#include <ovprintf.h>

struct put_char_context {
  wchar_t **const dest;
  size_t pos;
  struct ov_error *err;
  bool failed;
};

static void put_char(int c, void *ctx) {
  struct put_char_context *pcctx = (struct put_char_context *)ctx;
  if (pcctx->failed) {
    return;
  }

  size_t const current_cap = *pcctx->dest ? OV_ARRAY_CAPACITY(*pcctx->dest) : 0;
  if (pcctx->pos >= current_cap) {
    size_t const new_cap = current_cap < 32 ? 64 : current_cap * 2;
    if (!OV_ARRAY_GROW(pcctx->dest, new_cap, pcctx->err)) {
      pcctx->failed = true;
      OV_ERROR_TRACE(pcctx->err);
      return;
    }
  }

  (*pcctx->dest)[pcctx->pos++] = (wchar_t)c;
}

bool ov_vsprintf_wchar(wchar_t **const dest,
                       wchar_t const *const reference,
                       wchar_t const *const format,
                       struct ov_error *const err,
                       va_list valist) {
  if (!dest || !format) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }
  if (*dest) {
    OV_ARRAY_SET_LENGTH(*dest, 0);
  }
  return ov_vsprintf_append_wchar(dest, reference, format, err, valist);
}

bool ov_sprintf_wchar(wchar_t **const dest,
                      wchar_t const *const reference,
                      wchar_t const *const format,
                      struct ov_error *const err,
                      ...) {
  va_list valist;
  va_start(valist, err);
  bool ok = ov_vsprintf_wchar(dest, reference, format, err, valist);
  va_end(valist);
  if (!ok) {
    OV_ERROR_TRACE(err);
    return false;
  }
  return true;
}

bool ov_vsprintf_append_wchar(wchar_t **const dest,
                              wchar_t const *const reference,
                              wchar_t const *const format,
                              struct ov_error *const err,
                              va_list valist) {
  if (!dest || !format) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }

  size_t const existing_len = *dest ? OV_ARRAY_LENGTH(*dest) : 0;

  struct put_char_context ctx = {
      .dest = dest,
      .pos = existing_len,
      .err = err,
      .failed = false,
  };

  int const r = ov_vpprintf(put_char, &ctx, reference, format, valist);
  if (r == 0) {
    if (ctx.failed) {
      OV_ERROR_TRACE(err);
    } else {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
    }
    return false;
  }

  put_char(0, &ctx);
  if (ctx.failed) {
    OV_ERROR_TRACE(err);
    return false;
  }

  OV_ARRAY_SET_LENGTH(*dest, existing_len + (size_t)r);
  return true;
}

bool ov_sprintf_append_wchar(wchar_t **const dest,
                             wchar_t const *const reference,
                             wchar_t const *const format,
                             struct ov_error *const err,
                             ...) {
  va_list valist;
  va_start(valist, err);
  bool ok = ov_vsprintf_append_wchar(dest, reference, format, err, valist);
  va_end(valist);
  if (!ok) {
    OV_ERROR_TRACE(err);
    return false;
  }
  return true;
}
