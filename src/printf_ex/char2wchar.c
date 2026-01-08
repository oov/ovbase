#include <ovprintf_ex.h>

#include <ovarray.h>
#include <ovbase.h>
#include <ovprintf.h>

#include "../printf/util.h"

#include <assert.h>

struct put_context {
  wchar_t **const dest;
  size_t pos;
  bool failed;
  struct utf8_state state;
};

static void put_wchar(int const c, void *const ctx) {
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

  (*pcctx->dest)[pcctx->pos++] = (wchar_t)c;
}

static void char2wchar_callback(int const c, void *const ctx) {
  struct put_context *const pcctx = (struct put_context *)ctx;
  if (pcctx->failed) {
    return;
  }

  int_fast32_t const cp = utf8_state_feed(&pcctx->state, (uint8_t)c);
  if (cp >= 0) {
    emit_codepoint_as_wchar(put_wchar, ctx, cp);
  }
}

bool ov_vsprintf_append_char2wchar(wchar_t **const dest,
                                   struct ov_error *const err,
                                   char const *const reference,
                                   char const *const format,
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
  utf8_state_init(&ctx.state);

  bool success = false;

  int const r = ov_vpprintf_char(char2wchar_callback, &ctx, reference, format, valist);
  if (r == 0 && *format != '\0') {
    // ov_vpprintf_char returns 0 on format mismatch error (when format is not empty)
    OV_ERROR_SET(err, ov_error_type_generic, ov_error_generic_fail, "format string mismatch with reference");
    goto cleanup;
  }

  if (ctx.failed) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
    goto cleanup;
  }

  if (!utf8_state_is_complete(&ctx.state)) {
    OV_ERROR_SET(err, ov_error_type_generic, ov_error_generic_fail, "incomplete UTF-8 sequence");
    goto cleanup;
  }

  put_wchar(0, &ctx);
  if (ctx.failed) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
    goto cleanup;
  }

  // Set length to actual wchar_t count (excluding null terminator)
  OV_ARRAY_SET_LENGTH(*dest, ctx.pos - 1);
  success = true;

cleanup:
  if (!success) {
    if (was_null && *dest) {
      OV_ARRAY_DESTROY(dest);
    } else if (!was_null && *dest) {
      OV_ARRAY_SET_LENGTH(*dest, existing_len);
      (*dest)[existing_len] = L'\0';
    }
  }
  return success;
}

bool ov_sprintf_append_char2wchar(
    wchar_t **const dest, struct ov_error *const err, char const *const reference, char const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  bool const result = ov_vsprintf_append_char2wchar(dest, err, reference, format, valist);
  va_end(valist);
  return result;
}

bool ov_vsprintf_char2wchar(wchar_t **const dest,
                            struct ov_error *const err,
                            char const *const reference,
                            char const *const format,
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
  return ov_vsprintf_append_char2wchar(dest, err, reference, format, valist);
}

bool ov_sprintf_char2wchar(
    wchar_t **const dest, struct ov_error *const err, char const *const reference, char const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  bool const result = ov_vsprintf_char2wchar(dest, err, reference, format, valist);
  va_end(valist);
  return result;
}
