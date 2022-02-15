#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef CHAR_TYPE
#  define CHAR_TYPE wchar_t
#endif

#ifndef STR
#  define STR(str) L##str
#endif

#ifndef INT_TYPE
#  define INT_TYPE int64_t
#endif

#ifndef UINT_TYPE
#  define UINT_TYPE uint64_t
#endif

#ifndef FLOAT_TYPE
#  define FLOAT_TYPE double
#endif

#ifndef BUFFER_SIZE
#  define BUFFER_SIZE 32
#endif

#ifndef BUFFER_SIZE_FLOAT
#  define BUFFER_SIZE_FLOAT 64
#endif

#ifndef PREFIX
#  define PREFIX ov_
#endif

#ifndef POSTFIX
#  define POSTFIX CHAR_TYPE
#endif

#ifndef FUNCNAME
#  define FUNCNAME_3(prefix, funcname, postfix) prefix##funcname##postfix
#  define FUNCNAME_2(prefix, funcname, postfix) FUNCNAME_3(prefix, funcname, postfix)
#  define FUNCNAME(funcname) FUNCNAME_2(PREFIX, funcname, POSTFIX)
#endif
