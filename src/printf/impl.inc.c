#include <ovprintf.h>

#define NANOPRINTF_VISIBILITY_STATIC
#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_SNPRINTF_SAFE_EMPTY_STRING_ON_OVERFLOW 1
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_FMT_SPEC_OPT_STAR 0
#define NANOPRINTF_USE_ORDER_FORMAT_EXTENSION_SPECIFIERS 1
#include "nanoprintf.h"

#ifndef FUNCNAME
#  define FUNCNAME_3(prefix, funcname, postfix) prefix##funcname##postfix
#  define FUNCNAME_2(prefix, funcname, postfix) FUNCNAME_3(prefix, funcname, postfix)
#  define FUNCNAME(funcname) FUNCNAME_2(PREFIX, funcname, POSTFIX)
#endif

int FUNCNAME(vpprintf)(void (*putc)(int c, void *ctx),
                       void *ctx,
                       CHAR_TYPE const *const reference,
                       CHAR_TYPE const *const format,
                       va_list valist) {
  return npf_vpprintf(putc, ctx, reference, format, valist);
}

int FUNCNAME(pprintf)(
    void (*putc)(int c, void *ctx), void *ctx, CHAR_TYPE const *const reference, CHAR_TYPE const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  int const r = npf_vpprintf(putc, ctx, reference, format, valist);
  va_end(valist);
  return r;
}

int FUNCNAME(snprintf)(
    CHAR_TYPE *const dest, size_t destlen, CHAR_TYPE const *const reference, CHAR_TYPE const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  int r = npf_vsnprintf(dest, destlen, reference, format, valist);
  va_end(valist);
  return r;
}

int FUNCNAME(vsnprintf)(CHAR_TYPE *const dest,
                        size_t destlen,
                        CHAR_TYPE const *const reference,
                        CHAR_TYPE const *const format,
                        va_list valist) {
  return npf_vsnprintf(dest, destlen, reference, format, valist);
}

int FUNCNAME(printf_verify_format)(CHAR_TYPE const *const reference, CHAR_TYPE const *const format) {
  return npf_verify_format(reference, format);
}
