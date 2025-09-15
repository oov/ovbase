#include <ovprintf_ex.h>

#include <ovarray.h>
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
                        CHAR_TYPE const *const reference,
                        CHAR_TYPE const *const format,
                        va_list valist) {
  assert(dest != NULL && "dest must not be NULL");
  assert(format != NULL && "format must not be NULL");
  if (!dest || !format) {
    return false;
  }
  if (*dest) {
    OV_ARRAY_SET_LENGTH(*dest, 0);
  }
  return FUNCNAME(vsprintf_append)(dest, reference, format, valist);
}

bool FUNCNAME(sprintf)(CHAR_TYPE **const dest, CHAR_TYPE const *const reference, CHAR_TYPE const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  bool ok = FUNCNAME(vsprintf)(dest, reference, format, valist);
  va_end(valist);
  return ok;
}

bool FUNCNAME(vsprintf_append)(CHAR_TYPE **const dest,
                               CHAR_TYPE const *const reference,
                               CHAR_TYPE const *const format,
                               va_list valist) {
  assert(dest != NULL && "dest must not be NULL");
  assert(format != NULL && "format must not be NULL");
  if (!dest || !format) {
    return false;
  }

  bool const was_null = *dest == NULL;
  size_t const existing_len = OV_ARRAY_LENGTH(*dest);

  struct put_context ctx = {
      .dest = dest,
      .pos = existing_len,
      .failed = false,
  };

  int const r = OV_VPPRINTF(put, &ctx, reference, format, valist);
  if (r == 0) {
    ctx.failed = true;
    goto cleanup;
  }

  put(0, &ctx);
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

bool FUNCNAME(sprintf_append)(CHAR_TYPE **const dest,
                              CHAR_TYPE const *const reference,
                              CHAR_TYPE const *const format,
                              ...) {
  va_list valist;
  va_start(valist, format);
  bool ok = FUNCNAME(vsprintf_append)(dest, reference, format, valist);
  va_end(valist);
  return ok;
}
