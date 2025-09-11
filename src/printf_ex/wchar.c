#include <ovprintf_ex.h>

#include <ovarray.h>
#include <ovprintf.h>

struct put_wchar_context {
  wchar_t **const dest;
  size_t pos;
  bool failed;
};

static void put_wchar(int c, void *ctx) {
  struct put_wchar_context *pcctx = (struct put_wchar_context *)ctx;
  if (pcctx->failed) {
    return;
  }

  size_t const current_cap = *pcctx->dest ? OV_ARRAY_CAPACITY(*pcctx->dest) : 0;
  if (pcctx->pos >= current_cap) {
    size_t const new_cap = current_cap < 32 ? 64 : current_cap * 2;
    if (!OV_ARRAY_GROW(pcctx->dest, new_cap, NULL)) {
      pcctx->failed = true;
      return;
    }
  }

  (*pcctx->dest)[pcctx->pos++] = (wchar_t)c;
}

bool ov_vsprintf_wchar(wchar_t **const dest,
                       wchar_t const *const reference,
                       wchar_t const *const format,
                       va_list valist) {
  if (!dest || !format) {
    return false;
  }
  if (*dest) {
    OV_ARRAY_SET_LENGTH(*dest, 0);
  }
  return ov_vsprintf_append_wchar(dest, reference, format, valist);
}

bool ov_sprintf_wchar(wchar_t **const dest, wchar_t const *const reference, wchar_t const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  bool ok = ov_vsprintf_wchar(dest, reference, format, valist);
  va_end(valist);
  return ok;
}

bool ov_vsprintf_append_wchar(wchar_t **const dest,
                              wchar_t const *const reference,
                              wchar_t const *const format,
                              va_list valist) {
  if (!dest || !format) {
    return false;
  }

  bool const was_null = *dest == NULL;
  size_t const existing_len = OV_ARRAY_LENGTH(*dest);

  struct put_wchar_context ctx = {
      .dest = dest,
      .pos = existing_len,
      .failed = false,
  };

  int const r = ov_vpprintf(put_wchar, &ctx, reference, format, valist);
  if (r == 0) {
    ctx.failed = true;
    goto cleanup;
  }

  put_wchar(0, &ctx);
  if (ctx.failed) {
    goto cleanup;
  }

  OV_ARRAY_SET_LENGTH(*dest, existing_len + (size_t)r);

cleanup:
  if (ctx.failed) {
    if (was_null) {
      OV_ARRAY_DESTROY(dest);
    }
  }
  return !ctx.failed;
}

bool ov_sprintf_append_wchar(wchar_t **const dest, wchar_t const *const reference, wchar_t const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  bool ok = ov_vsprintf_append_wchar(dest, reference, format, valist);
  va_end(valist);
  return ok;
}
