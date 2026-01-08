#include <ovprintf_ex.h>

#include <ovarray.h>
#include <ovbase.h>
#include <ovprintf.h>

#include "../printf/util.h"

#include <assert.h>

struct put_context {
  char **const dest;
  size_t pos;
  bool failed;
  struct wchar_state state;
};

static void put_char(int const c, void *const ctx) {
  struct put_context *const pcctx = (struct put_context *)ctx;
  if (pcctx->failed) {
    return;
  }

  size_t const current_cap = *pcctx->dest ? OV_ARRAY_CAPACITY(*pcctx->dest) : 0;
  if (pcctx->pos >= current_cap) {
    size_t const new_cap = current_cap < 32 ? 64 : current_cap * 2;
    if (!OV_ARRAY_GROW(pcctx->dest, new_cap)) {
      pcctx->failed = true;
      return;
    }
  }

  (*pcctx->dest)[pcctx->pos++] = (char)c;
}

static void wchar2char_callback(int const c, void *const ctx) {
  struct put_context *const pcctx = (struct put_context *)ctx;
  if (pcctx->failed) {
    return;
  }

  int_fast32_t const cp = wchar_state_feed(&pcctx->state, (wchar_t)c);
  if (cp >= 0) {
    emit_codepoint_as_utf8(put_char, ctx, cp);
  }
}

bool ov_vsprintf_append_wchar2char(char **const dest,
                                   struct ov_error *const err,
                                   wchar_t const *const reference,
                                   wchar_t const *const format,
                                   va_list valist) {
  assert(dest != NULL && "dest must not be NULL");
  assert(format != NULL && "format must not be NULL");
  if (!dest || !format) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }

  bool const was_null = *dest == NULL;
  size_t const existing_len = OV_ARRAY_LENGTH(*dest);

  struct put_context ctx = {
      .dest = dest,
      .pos = existing_len,
      .failed = false,
  };
  wchar_state_init(&ctx.state);

  bool success = false;

  int const r = ov_vpprintf_wchar(wchar2char_callback, &ctx, reference, format, valist);
  if (r == 0 && *format != L'\0') {
    // ov_vpprintf_wchar returns 0 on format mismatch error (when format is not empty)
    OV_ERROR_SET(err, ov_error_type_generic, ov_error_generic_fail, "format string mismatch with reference");
    goto cleanup;
  }

  if (ctx.failed) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
    goto cleanup;
  }

  if (!wchar_state_is_complete(&ctx.state)) {
    OV_ERROR_SET(err, ov_error_type_generic, ov_error_generic_fail, "incomplete surrogate pair");
    goto cleanup;
  }

  put_char(0, &ctx);
  if (ctx.failed) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
    goto cleanup;
  }

  // Set length to actual char count (excluding null terminator)
  OV_ARRAY_SET_LENGTH(*dest, ctx.pos - 1);
  success = true;

cleanup:
  if (!success) {
    if (was_null && *dest) {
      OV_ARRAY_DESTROY(dest);
    } else if (!was_null && *dest) {
      OV_ARRAY_SET_LENGTH(*dest, existing_len);
      (*dest)[existing_len] = '\0';
    }
  }
  return success;
}

bool ov_sprintf_append_wchar2char(
    char **const dest, struct ov_error *const err, wchar_t const *const reference, wchar_t const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  bool const result = ov_vsprintf_append_wchar2char(dest, err, reference, format, valist);
  va_end(valist);
  return result;
}

bool ov_vsprintf_wchar2char(char **const dest,
                            struct ov_error *const err,
                            wchar_t const *const reference,
                            wchar_t const *const format,
                            va_list valist) {
  assert(dest != NULL && "dest must not be NULL");
  assert(format != NULL && "format must not be NULL");
  if (!dest || !format) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }
  if (*dest) {
    OV_ARRAY_SET_LENGTH(*dest, 0);
  }
  return ov_vsprintf_append_wchar2char(dest, err, reference, format, valist);
}

bool ov_sprintf_wchar2char(
    char **const dest, struct ov_error *const err, wchar_t const *const reference, wchar_t const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  bool const result = ov_vsprintf_wchar2char(dest, err, reference, format, valist);
  va_end(valist);
  return result;
}
