#include <ovprintf_ex.h>

#include <ovarray.h>
#include <ovbase.h>
#include <ovprintf.h>

#include <assert.h>

#ifndef CHAR_TYPE
#  error "CHAR_TYPE must be defined before including this file"
#endif

#ifndef PREFIX
#  error "PREFIX must be defined before including this file"
#endif

#ifndef POSTFIX
#  error "POSTFIX must be defined before including this file"
#endif

#ifndef FUNCNAME
#  define FUNCNAME_3(prefix, funcname, postfix) prefix##funcname##postfix
#  define FUNCNAME_2(prefix, funcname, postfix) FUNCNAME_3(prefix, funcname, postfix)
#  define FUNCNAME(funcname) FUNCNAME_2(PREFIX, funcname, POSTFIX)
#endif

struct put_context {
  CHAR_TYPE **const dest;
  size_t pos;
  bool failed;
};

static void put(int c, void *ctx) {
  struct put_context *pcctx = (struct put_context *)ctx;
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

  (*pcctx->dest)[pcctx->pos++] = (CHAR_TYPE)c;
}

bool FUNCNAME(vsprintf)(CHAR_TYPE **const dest,
                        struct ov_error *const err,
                        CHAR_TYPE const *const reference,
                        CHAR_TYPE const *const format,
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
  return FUNCNAME(vsprintf_append)(dest, err, reference, format, valist);
}

bool FUNCNAME(sprintf)(CHAR_TYPE **const dest,
                       struct ov_error *const err,
                       CHAR_TYPE const *const reference,
                       CHAR_TYPE const *const format,
                       ...) {
  va_list valist;
  va_start(valist, format);
  bool const result = FUNCNAME(vsprintf)(dest, err, reference, format, valist);
  va_end(valist);
  return result;
}

bool FUNCNAME(vsprintf_append)(CHAR_TYPE **const dest,
                               struct ov_error *const err,
                               CHAR_TYPE const *const reference,
                               CHAR_TYPE const *const format,
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

  bool success = false;

  int const r = OV_VPPRINTF(put, &ctx, reference, format, valist);
  if (r == 0 && *format != '\0') {
    // OV_VPPRINTF returns 0 on format mismatch error (when format is not empty)
    // An empty format string legitimately produces 0 characters
    OV_ERROR_SET(err, ov_error_type_generic, ov_error_generic_fail, "format string mismatch with reference");
    goto cleanup;
  }

  if (ctx.failed) {
    // Memory allocation failed in put()
    OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
    goto cleanup;
  }

  put(0, &ctx);
  if (ctx.failed) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
    goto cleanup;
  }

  OV_ARRAY_SET_LENGTH(*dest, existing_len + (size_t)r);
  success = true;

cleanup:
  if (!success) {
    if (was_null && *dest) {
      OV_ARRAY_DESTROY(dest);
    } else if (!was_null && *dest) {
      // Restore original length on failure
      OV_ARRAY_SET_LENGTH(*dest, existing_len);
      (*dest)[existing_len] = '\0';
    }
  }
  return success;
}

bool FUNCNAME(sprintf_append)(CHAR_TYPE **const dest,
                              struct ov_error *const err,
                              CHAR_TYPE const *const reference,
                              CHAR_TYPE const *const format,
                              ...) {
  va_list valist;
  va_start(valist, format);
  bool const result = FUNCNAME(vsprintf_append)(dest, err, reference, format, valist);
  va_end(valist);
  return result;
}
