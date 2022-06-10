#include "ovprintf.h"

#define NANOPRINTF_VISIBILITY_STATIC
#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_SNPRINTF_SAFE_EMPTY_STRING_ON_OVERFLOW 1
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#include NANOPRINTF_HEADER_FILE

#ifndef FUNCNAME
#  define FUNCNAME_3(prefix, funcname, postfix) prefix##funcname##postfix
#  define FUNCNAME_2(prefix, funcname, postfix) FUNCNAME_3(prefix, funcname, postfix)
#  define FUNCNAME(funcname) FUNCNAME_2(PREFIX, funcname, POSTFIX)
#endif

int FUNCNAME(snprintf)(CHAR_TYPE *const dest, size_t destlen, CHAR_TYPE const *const format, ...) {
  va_list valist;
  va_start(valist, format);
  int r = npf_vsnprintf(dest, destlen, format, valist);
  va_end(valist);
  return r;
}

int FUNCNAME(vsnprintf)(CHAR_TYPE *const dest, size_t destlen, CHAR_TYPE const *const format, va_list valist) {
  return npf_vsnprintf(dest, destlen, format, valist);
}
