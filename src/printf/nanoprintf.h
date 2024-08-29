/* nanoprintf: a tiny embeddable printf replacement written in C.
   https://github.com/charlesnicholson/nanoprintf
   charles.nicholson+nanoprintf@gmail.com
   dual-licensed under 0bsd AND unlicense, see end of file for details. */

#ifndef NANOPRINTF_H_INCLUDED
#  define NANOPRINTF_H_INCLUDED

#  include <stdarg.h>
#  include <stddef.h>

#  ifdef NANOPRINTF_USE_WCHAR
#    include <wchar.h>
#    define NPF_CHAR_TYPE wchar_t
#  else
#    define NPF_CHAR_TYPE char
#  endif

// Define this to fully sandbox nanoprintf inside of a translation unit.
#  ifdef NANOPRINTF_VISIBILITY_STATIC
#    define NPF_VISIBILITY static
#  else
#    define NPF_VISIBILITY extern
#  endif

#  if 0
#    if !defined(NANOPRINTF_USE_WCHAR) && (defined(__clang__) || defined(__GNUC__) || defined(__GNUG__))
#      define NPF_PRINTF_ATTR(FORMAT_INDEX, VARGS_INDEX) __attribute__((format(printf, FORMAT_INDEX, VARGS_INDEX)))
#    else
#      define NPF_PRINTF_ATTR(FORMAT_INDEX, VARGS_INDEX)
#    endif
#  else
#    define NPF_PRINTF_ATTR(FORMAT_INDEX, VARGS_INDEX)
#  endif

// Public API

#  ifdef __cplusplus
extern "C" {
#  endif

NPF_VISIBILITY int
npf_snprintf(NPF_CHAR_TYPE *buffer, size_t bufsz, NPF_CHAR_TYPE const *reference, const NPF_CHAR_TYPE *format, ...)
    NPF_PRINTF_ATTR(3, 4);

NPF_VISIBILITY int npf_vsnprintf(NPF_CHAR_TYPE *buffer,
                                 size_t bufsz,
                                 NPF_CHAR_TYPE const *reference,
                                 NPF_CHAR_TYPE const *format,
                                 va_list vlist) NPF_PRINTF_ATTR(3, 0);

typedef void (*npf_putc)(int c, void *ctx);
NPF_VISIBILITY int
npf_pprintf(npf_putc pc, void *pc_ctx, NPF_CHAR_TYPE const *reference, NPF_CHAR_TYPE const *format, ...)
    NPF_PRINTF_ATTR(3, 4);

NPF_VISIBILITY int
npf_vpprintf(npf_putc pc, void *pc_ctx, NPF_CHAR_TYPE const *reference, NPF_CHAR_TYPE const *format, va_list vlist)
    NPF_PRINTF_ATTR(3, 0);

NPF_VISIBILITY int npf_verify_format(NPF_CHAR_TYPE const *reference, NPF_CHAR_TYPE const *format);

#  ifdef __cplusplus
}
#  endif

#endif // NANOPRINTF_H_INCLUDED

/* The implementation of nanoprintf begins here, to be compiled only if
   NANOPRINTF_IMPLEMENTATION is defined. In a multi-file library what follows would
   be nanoprintf.c. */

#ifdef NANOPRINTF_IMPLEMENTATION

#  ifndef NANOPRINTF_IMPLEMENTATION_INCLUDED
#    define NANOPRINTF_IMPLEMENTATION_INCLUDED

#    include "../../include/ovutf.h"
#    include <inttypes.h>
#    include <stdint.h>

// Pick reasonable defaults if nothing's been configured.
#    if !defined(NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS) &&                                                      \
        !defined(NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS) && !defined(NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS) &&    \
        !defined(NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS) && !defined(NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS) &&       \
        !defined(NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS) && !defined(NANOPRINTF_USE_FMT_SPEC_OPT_STAR) &&          \
        !defined(NANOPRINTF_USE_ORDER_FORMAT_EXTENSION_SPECIFIERS)
#      define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#      define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#      define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 1
#      define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#      define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#      define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#      define NANOPRINTF_USE_FMT_SPEC_OPT_STAR 1
#      define NANOPRINTF_USE_ORDER_FORMAT_EXTENSION_SPECIFIERS 0
#    endif

// If anything's been configured, everything must be configured.
#    ifndef NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS
#      error NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS must be #defined to 0 or 1
#    endif
#    ifndef NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS
#      error NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS must be #defined to 0 or 1
#    endif
#    ifndef NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS
#      error NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS must be #defined to 0 or 1
#    endif
#    ifndef NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS
#      error NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS must be #defined to 0 or 1
#    endif
#    ifndef NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS
#      error NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS must be #defined to 0 or 1
#    endif
#    ifndef NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS
#      error NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS must be #defined to 0 or 1
#    endif
#    ifndef NANOPRINTF_USE_FMT_SPEC_OPT_STAR
#      error NANOPRINTF_USE_FMT_SPEC_OPT_STAR must be #defined to 0 or 1
#    endif
#    ifndef NANOPRINTF_USE_ORDER_FORMAT_EXTENSION_SPECIFIERS
#      error NANOPRINTF_USE_ORDER_FORMAT_EXTENSION_SPECIFIERS must be #defined to 0 or 1
#    endif

// Ensure flags are compatible.
#    if (NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1) && (NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 0)
#      error Precision format specifiers must be enabled if float support is enabled.
#    endif

#    if defined(NANOPRINTF_SNPRINTF_SAFE_EMPTY_STRING_ON_OVERFLOW) &&                                                  \
        defined(NANOPRINTF_SNPRINTF_SAFE_TRIM_STRING_ON_OVERFLOW)
#      error snprintf safety flags are mutually exclusive.
#    endif

// intmax_t / uintmax_t require stdint from c99 / c++11
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
#      ifndef _MSC_VER
#        ifdef __cplusplus
#          if __cplusplus < 201103L
#            error large format specifier support requires C++11 or later.
#          endif
#        else
#          if __STDC_VERSION__ < 199409L
#            error nanoprintf requires C99 or later.
#          endif
#        endif
#      endif
#    endif

// Figure out if we can disable warnings with pragmas.
#    ifdef __clang__
#      define NANOPRINTF_CLANG 1
#      define NANOPRINTF_GCC_PAST_4_6 0
#    else
#      define NANOPRINTF_CLANG 0
#      if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 6)))
#        define NANOPRINTF_GCC_PAST_4_6 1
#      else
#        define NANOPRINTF_GCC_PAST_4_6 0
#      endif
#    endif

#    if NANOPRINTF_CLANG || NANOPRINTF_GCC_PAST_4_6
#      define NANOPRINTF_HAVE_GCC_WARNING_PRAGMAS 1
#    else
#      define NANOPRINTF_HAVE_GCC_WARNING_PRAGMAS 0
#    endif

#    if NANOPRINTF_HAVE_GCC_WARNING_PRAGMAS
#      pragma GCC diagnostic push
#      pragma GCC diagnostic ignored "-Wunused-function"
#      pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#      ifdef __cplusplus
#        pragma GCC diagnostic ignored "-Wold-style-cast"
#      endif
#      pragma GCC diagnostic ignored "-Wpadded"
#      pragma GCC diagnostic ignored "-Wfloat-equal"
#      if NANOPRINTF_CLANG
#        pragma GCC diagnostic ignored "-Wc++98-compat-pedantic"
#        pragma GCC diagnostic ignored "-Wcovered-switch-default"
#        pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#      elif NANOPRINTF_GCC_PAST_4_6
#        pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#      endif
#    endif

#    ifdef _MSC_VER
#      pragma warning(push)
#      pragma warning(disable : 4514) // unreferenced inline function removed
#      pragma warning(disable : 4505) // unreferenced function removed
#      pragma warning(disable : 4820) // padding after data member
#      pragma warning(disable : 5039) // extern "C" throw
#      pragma warning(disable : 5045) // spectre mitigation
#      pragma warning(disable : 4701) // possibly uninitialized
#      pragma warning(disable : 4706) // assignment in conditional
#      pragma warning(disable : 4710) // not inlined
#      pragma warning(disable : 4711) // selected for inline
#    endif

#    if (NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1) || (NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1)
typedef enum {
  NPF_FMT_SPEC_OPT_NONE,
  NPF_FMT_SPEC_OPT_LITERAL,
#      if NANOPRINTF_USE_FMT_SPEC_OPT_STAR == 1
  NPF_FMT_SPEC_OPT_STAR,
#      endif
} npf_fmt_spec_opt_t;
#    endif

typedef enum {
  NPF_FMT_SPEC_LEN_MOD_NONE,
  NPF_FMT_SPEC_LEN_MOD_SHORT,       // 'h'
  NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE, // 'L'
  NPF_FMT_SPEC_LEN_MOD_CHAR,        // 'hh'
  NPF_FMT_SPEC_LEN_MOD_LONG         // 'l'
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
  ,
  NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG, // 'll'
  NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX,    // 'j'
  NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET,     // 'z'
  NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT   // 't'
#    endif
} npf_format_spec_length_modifier_t;

typedef enum {
  NPF_FMT_SPEC_CONV_PERCENT,    // '%'
  NPF_FMT_SPEC_CONV_CHAR,       // 'c'
  NPF_FMT_SPEC_CONV_STRING,     // 's'
  NPF_FMT_SPEC_CONV_SIGNED_INT, // 'i', 'd'
#    if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
  NPF_FMT_SPEC_CONV_BINARY, // 'b'
#    endif
  NPF_FMT_SPEC_CONV_OCTAL,        // 'o'
  NPF_FMT_SPEC_CONV_HEX_INT,      // 'x', 'X'
  NPF_FMT_SPEC_CONV_UNSIGNED_INT, // 'u'
  NPF_FMT_SPEC_CONV_POINTER       // 'p'
#    if NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS == 1
  ,
  NPF_FMT_SPEC_CONV_WRITEBACK // 'n'
#    endif
#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
  ,
  NPF_FMT_SPEC_CONV_FLOAT_DECIMAL // 'f', 'F'
#    endif
} npf_format_spec_conversion_t;

typedef struct {
  int order;
  NPF_CHAR_TYPE prepend;  // ' ' or '+'
  NPF_CHAR_TYPE alt_form; // '#'

#    if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
  npf_fmt_spec_opt_t field_width_opt;
  int field_width;
  NPF_CHAR_TYPE left_justified;   // '-'
  NPF_CHAR_TYPE leading_zero_pad; // '0'
#    endif

#    if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
  npf_fmt_spec_opt_t prec_opt;
  int prec;
#    endif

  npf_format_spec_length_modifier_t length_modifier;
  npf_format_spec_conversion_t conv_spec;
  NPF_CHAR_TYPE case_adjust;
} npf_format_spec_t;

#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 0
typedef long npf_int_t;
typedef unsigned long npf_uint_t;
#    else
typedef intmax_t npf_int_t;
typedef uintmax_t npf_uint_t;
#    endif

typedef struct {
  NPF_CHAR_TYPE *dst;
  size_t len;
  size_t cur;
} npf_bufputc_ctx_t;

static int npf_parse_format_spec(NPF_CHAR_TYPE const *format, npf_format_spec_t *out_spec);
static void npf_bufputc(int c, void *ctx);
static void npf_bufputc_nop(int c, void *ctx);
static int npf_itoa_rev(NPF_CHAR_TYPE *buf, npf_int_t i);
static int npf_utoa_rev(NPF_CHAR_TYPE *buf, npf_uint_t i, unsigned base, unsigned case_adjust);

#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
static int npf_fsplit_abs(float f, uint64_t *out_int_part, uint64_t *out_frac_part, int *out_frac_base10_neg_e);
static int npf_ftoa_rev(NPF_CHAR_TYPE *buf, float f, NPF_CHAR_TYPE case_adj, int *out_frac_chars);
#    endif

#    if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
static int npf_bin_len(npf_uint_t i);
#    endif

#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
#      ifdef _MSC_VER
#        include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#      else
#        include <sys/types.h>
#      endif
#    endif

#    ifdef _MSC_VER
#      include <intrin.h>
#    endif

static int npf_min(int x, int y) { return (x < y) ? x : y; }
static int npf_max(int x, int y) { return (x > y) ? x : y; }

int npf_parse_format_spec(NPF_CHAR_TYPE const *format, npf_format_spec_t *out_spec) {
  NPF_CHAR_TYPE const *cur = format;
  out_spec->order = 0;

#    if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
  out_spec->left_justified = 0;
  out_spec->leading_zero_pad = 0;
#    endif
  out_spec->case_adjust = 'a' - 'A'; // lowercase
  out_spec->prepend = 0;
  out_spec->alt_form = 0;

  switch (*++cur) { // cur points at the leading '%' character
#    if NANOPRINTF_USE_ORDER_FORMAT_EXTENSION_SPECIFIERS == 1
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    while ((*cur >= '0') && (*cur <= '9')) {
      out_spec->order = (out_spec->order * 10) + (*cur++ - '0');
    }
    if (*cur == '$') {
      if (out_spec->order == 0) {
        return 0;
      }
    } else {
      cur = format;
      out_spec->order = 0;
    }
    break;
#    endif
  default:
    --cur;
    break;
  }

  while (*++cur) {
    switch (*cur) { // Optional flags
#    if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    case '-':
      out_spec->left_justified = '-';
      out_spec->leading_zero_pad = 0;
      continue;
    case '0':
      out_spec->leading_zero_pad = !out_spec->left_justified;
      continue;
#    endif
    case '+':
      out_spec->prepend = '+';
      continue;
    case ' ':
      if (out_spec->prepend == 0) {
        out_spec->prepend = ' ';
      }
      continue;
    case '#':
      out_spec->alt_form = '#';
      continue;
    default:
      break;
    }
    break;
  }

#    if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
  out_spec->field_width = 0;
  out_spec->field_width_opt = NPF_FMT_SPEC_OPT_NONE;
#      if NANOPRINTF_USE_FMT_SPEC_OPT_STAR == 1
  if (*cur == '*') {
    out_spec->field_width_opt = NPF_FMT_SPEC_OPT_STAR;
    ++cur;
#        if NANOPRINTF_USE_ORDER_FORMAT_EXTENSION_SPECIFIERS == 1
    switch (*cur) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      while ((*cur >= '0') && (*cur <= '9')) {
        out_spec->field_width = (out_spec->field_width * 10) + (*cur++ - '0');
      }
      if (*cur != '$' || out_spec->field_width == 0) {
        return 0;
      }
    }
#        endif
  } else
#      endif
  {
    while ((*cur >= '0') && (*cur <= '9')) {
      out_spec->field_width_opt = NPF_FMT_SPEC_OPT_LITERAL;
      out_spec->field_width = (out_spec->field_width * 10) + (*cur++ - '0');
    }
  }
#    endif

#    if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
  out_spec->prec = 0;
  out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
  if (*cur == '.') {
    ++cur;
#      if NANOPRINTF_USE_FMT_SPEC_OPT_STAR == 1
    if (*cur == '*') {
      out_spec->prec_opt = NPF_FMT_SPEC_OPT_STAR;
      ++cur;
#        if NANOPRINTF_USE_ORDER_FORMAT_EXTENSION_SPECIFIERS == 1
      switch (*cur) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        while ((*cur >= '0') && (*cur <= '9')) {
          out_spec->prec = (out_spec->prec * 10) + (*cur++ - '0');
        }
        if (*cur != '$' || out_spec->prec == 0) {
          return 0;
        }
      }
#        endif
    } else
#      endif
    {
      if (*cur == '-') {
        ++cur;
        out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
      } else {
        out_spec->prec_opt = NPF_FMT_SPEC_OPT_LITERAL;
      }
      while ((*cur >= '0') && (*cur <= '9')) {
        out_spec->prec = (out_spec->prec * 10) + (*cur++ - '0');
      }
    }
  }
#    endif

  out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_NONE;
  switch (*cur++) { // Length modifier
  case 'h':
    out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_SHORT;
    if (*cur == 'h') {
      out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_CHAR;
      ++cur;
    }
    break;
  case 'l':
    out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LONG;
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
    if (*cur == 'l') {
      out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG;
      ++cur;
    }
#    endif
    break;
#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
  case 'L':
    out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE;
    break;
#    endif
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
  case 'j':
    out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX;
    break;
  case 'z':
    out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET;
    break;
  case 't':
    out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT;
    break;
#    endif
  default:
    --cur;
    break;
  }

  switch (*cur++) { // Conversion specifier
  case '%':
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_PERCENT;
#    if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
    out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
#    endif
    break;
  case 'c':
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_CHAR;
#    if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
    out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
#    endif
    break;
  case 's':
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_STRING;
#    if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    out_spec->leading_zero_pad = 0;
#    endif
    break;

  case 'i':
  case 'd':
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_SIGNED_INT;
    break;

  case 'o':
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_OCTAL;
    break;
  case 'u':
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_UNSIGNED_INT;
    break;

  case 'X':
    out_spec->case_adjust = 0;
  case 'x':
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_HEX_INT;
    break;

#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
  case 'F':
    out_spec->case_adjust = 0;
  case 'f':
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_FLOAT_DECIMAL;
    if (out_spec->prec_opt == NPF_FMT_SPEC_OPT_NONE) {
      out_spec->prec = 6;
    }
    break;
#    endif

#    if NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS == 1
  case 'n':
    // todo: reject string if flags or width or precision exist
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_WRITEBACK;
#      if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
    out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
#      endif
    break;
#    endif

  case 'p':
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_POINTER;
#    if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
    out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
#    endif
    break;

#    if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
  case 'B':
    out_spec->case_adjust = 0;
  case 'b':
    out_spec->conv_spec = NPF_FMT_SPEC_CONV_BINARY;
    break;
#    endif

  default:
    return 0;
  }

  return (int)(cur - format);
}

int npf_itoa_rev(NPF_CHAR_TYPE *buf, npf_int_t i) {
  int n = 0;
  int const sign = (i >= 0) ? 1 : -1;
  do {
    *buf++ = (NPF_CHAR_TYPE)('0' + (sign * (i % 10)));
    i /= 10;
    ++n;
  } while (i);
  return n;
}

int npf_utoa_rev(NPF_CHAR_TYPE *buf, npf_uint_t i, unsigned base, unsigned case_adj) {
  int n = 0;
  do {
    unsigned const d = (unsigned)(i % base);
    *buf++ = (NPF_CHAR_TYPE)((d < 10) ? ('0' + d) : ('A' + case_adj + (d - 10)));
    i /= base;
    ++n;
  } while (i);
  return n;
}

#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
enum {
  NPF_MANTISSA_BITS = 23,
  NPF_EXPONENT_BITS = 8,
  NPF_EXPONENT_BIAS = 127,
  NPF_FRACTION_BIN_DIGITS = 64,
  NPF_MAX_FRACTION_DEC_DIGITS = 8
};

int npf_fsplit_abs(float f, uint64_t *out_int_part, uint64_t *out_frac_part, int *out_frac_base10_neg_exp) {
  /* conversion algorithm by Wojciech Muła (zdjęcia@garnek.pl)
     http://0x80.pl/notesen/2015-12-29-float-to-string.html
     grisu2 (https://bit.ly/2JgMggX) and ryu (https://bit.ly/2RLXSg0)
     are fast + precise + round, but require large lookup tables. */

  uint32_t f_bits;
  { // union-cast is UB, let compiler optimize byte-copy loop.
    NPF_CHAR_TYPE const *src = (NPF_CHAR_TYPE const *)&f;
    NPF_CHAR_TYPE *dst = (NPF_CHAR_TYPE *)&f_bits;
    for (unsigned i = 0; i < sizeof(f_bits); ++i) {
      dst[i] = src[i];
    }
  }

  int const exponent =
      ((int)((f_bits >> NPF_MANTISSA_BITS) & ((1u << NPF_EXPONENT_BITS) - 1u)) - NPF_EXPONENT_BIAS) - NPF_MANTISSA_BITS;

  if (exponent >= (64 - NPF_MANTISSA_BITS)) {
    return 0;
  } // value is out of range

  uint32_t const implicit_one = 1u << NPF_MANTISSA_BITS;
  uint32_t const mantissa = f_bits & (implicit_one - 1);
  uint32_t const mantissa_norm = mantissa | implicit_one;

  if (exponent > 0) {
    *out_int_part = (uint64_t)mantissa_norm << exponent;
  } else if (exponent < 0) {
    if (-exponent > NPF_MANTISSA_BITS) {
      *out_int_part = 0;
    } else {
      *out_int_part = mantissa_norm >> -exponent;
    }
  } else {
    *out_int_part = mantissa_norm;
  }

  uint64_t frac;
  {
    int const shift = NPF_FRACTION_BIN_DIGITS + exponent - 4;
    if ((shift >= (NPF_FRACTION_BIN_DIGITS - 4)) || (shift < 0)) {
      frac = 0;
    } else {
      frac = ((uint64_t)mantissa_norm) << shift;
    }
    // multiply off the leading one's digit
    frac &= 0x0fffffffffffffffllu;
    frac *= 10;
  }

  { // Count the number of 0s at the beginning of the fractional part.
    int frac_base10_neg_exp = 0;
    while (frac && ((frac >> (NPF_FRACTION_BIN_DIGITS - 4))) == 0) {
      ++frac_base10_neg_exp;
      frac &= 0x0fffffffffffffffllu;
      frac *= 10;
    }
    *out_frac_base10_neg_exp = frac_base10_neg_exp;
  }

  { // Convert the fractional part to base 10.
    unsigned frac_part = 0;
    for (int i = 0; frac && (i < NPF_MAX_FRACTION_DEC_DIGITS); ++i) {
      frac_part *= 10;
      frac_part += (unsigned)(frac >> (NPF_FRACTION_BIN_DIGITS - 4));
      frac &= 0x0fffffffffffffffllu;
      frac *= 10;
    }
    *out_frac_part = frac_part;
  }
  return 1;
}

int npf_ftoa_rev(NPF_CHAR_TYPE *buf, float f, NPF_CHAR_TYPE case_adj, int *out_frac_chars) {
  uint32_t f_bits;
  { // union-cast is UB, let compiler optimize byte-copy loop.
    NPF_CHAR_TYPE const *src = (NPF_CHAR_TYPE const *)&f;
    NPF_CHAR_TYPE *dst = (NPF_CHAR_TYPE *)&f_bits;
    for (unsigned i = 0; i < sizeof(f_bits); ++i) {
      dst[i] = src[i];
    }
  }

  if ((uint8_t)(f_bits >> 23) == 0xFF) {
    if (f_bits & 0x7fffff) {
      for (int i = 0; i < 3; ++i) {
        *buf++ = (NPF_CHAR_TYPE)("NAN"[i] + case_adj);
      }
    } else {
      for (int i = 0; i < 3; ++i) {
        *buf++ = (NPF_CHAR_TYPE)("FNI"[i] + case_adj);
      }
    }
    return -3;
  }

  uint64_t int_part, frac_part;
  int frac_base10_neg_exp;
  if (npf_fsplit_abs(f, &int_part, &frac_part, &frac_base10_neg_exp) == 0) {
    for (int i = 0; i < 3; ++i) {
      *buf++ = (NPF_CHAR_TYPE)("ROO"[i] + case_adj);
    }
    return -3;
  }

  NPF_CHAR_TYPE *dst = buf;

  while (frac_part) { // write the fractional digits
    *dst++ = (NPF_CHAR_TYPE)('0' + (frac_part % 10));
    frac_part /= 10;
  }

  // write the 0 digits between the . and the first fractional digit
  while (frac_base10_neg_exp-- > 0) {
    *dst++ = '0';
  }
  *out_frac_chars = (int)(dst - buf);
  *dst++ = '.';

  // write the integer digits
  do {
    *dst++ = (NPF_CHAR_TYPE)('0' + (int_part % 10));
    int_part /= 10;
  } while (int_part);
  return (int)(dst - buf);
}

#    endif // NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS

#    if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
int npf_bin_len(npf_uint_t u) {
  // Return the length of the binary string format of 'u', preferring intrinsics.
  if (!u) {
    return 1;
  }

#      ifdef _MSC_VER // Win64, use _BSR64 for everything. If x86, use _BSR when non-large.
#        ifdef _M_X64
#          define NPF_HAVE_BUILTIN_CLZ
#          define NPF_CLZ _BitScanReverse64
#        elif NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 0
#          define NPF_HAVE_BUILTIN_CLZ
#          define NPF_CLZ _BitScanReverse
#        endif
#        ifdef NPF_HAVE_BUILTIN_CLZ
  unsigned long idx;
  NPF_CLZ(&idx, u);
  return (int)(idx + 1);
#        endif
#      elif defined(NANOPRINTF_CLANG) || defined(NANOPRINTF_GCC_PAST_4_6)
#        define NPF_HAVE_BUILTIN_CLZ
#        if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
#          define NPF_CLZ(X) ((sizeof(long long) * 8) - (size_t)__builtin_clzll(X))
#        else
#          define NPF_CLZ(X) ((sizeof(long) * 8) - (size_t)__builtin_clzl(X))
#        endif
  return (int)NPF_CLZ(u);
#      endif

#      ifndef NPF_HAVE_BUILTIN_CLZ
  int n;
  for (n = 0; u; ++n, u >>= 1)
    ; // slow but small software fallback
  return n;
#      else
#        undef NPF_HAVE_BUILTIN_CLZ
#        undef NPF_CLZ
#      endif
}
#    endif

struct npf_arg_type {
  npf_format_spec_conversion_t conv_spec;
  npf_format_spec_length_modifier_t length_modifier;
};

union npf_arg_value {
  char *hstr;
  wchar_t *lstr;
  int i;
  long l;
  unsigned u;
  unsigned long ul;
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
  long long ll;
  intmax_t imx;
  ssize_t ssz;
  ptrdiff_t ptrdiff;
  unsigned long long ull;
  uintmax_t uimx;
  size_t sz;
#    endif
  void *ptr;
#    if NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS == 1
  int *ip;
  short *sp;
  double *dp;
  char *cp;
  long *lp;
#      if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
  long long *llp;
  intmax_t *imxp;
  ssize_t *ssz;
  ptrdiff_t *ptrdiffp;
#      endif
#    endif
#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
  long double ld;
  double d;
#    endif
};

static size_t npf_arg_sizeof(npf_format_spec_conversion_t const conv_spec,
                             npf_format_spec_length_modifier_t const length_modifier) {
  switch (conv_spec) {
  case NPF_FMT_SPEC_CONV_PERCENT:
    return 0;
  case NPF_FMT_SPEC_CONV_CHAR:
    return sizeof(int);
  case NPF_FMT_SPEC_CONV_STRING:
    if (length_modifier == NPF_FMT_SPEC_LEN_MOD_NONE || length_modifier == NPF_FMT_SPEC_LEN_MOD_SHORT) {
      return sizeof(char *);
    }
    if (length_modifier == NPF_FMT_SPEC_LEN_MOD_LONG) {
      return sizeof(wchar_t *);
    }
    return 0;
  case NPF_FMT_SPEC_CONV_SIGNED_INT:
    switch (length_modifier) {
    case NPF_FMT_SPEC_LEN_MOD_NONE:
    case NPF_FMT_SPEC_LEN_MOD_SHORT:
    case NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE:
    case NPF_FMT_SPEC_LEN_MOD_CHAR:
      return sizeof(int);
    case NPF_FMT_SPEC_LEN_MOD_LONG:
      return sizeof(long);
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
    case NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG:
      return sizeof(long long);
    case NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX:
      return sizeof(intmax_t);
    case NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET:
      return sizeof(ssize_t);
    case NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT:
      return sizeof(ptrdiff_t);
#    endif
    default:
      return 0;
    }
#    if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
  case NPF_FMT_SPEC_CONV_BINARY:
#    endif
  case NPF_FMT_SPEC_CONV_OCTAL:
  case NPF_FMT_SPEC_CONV_HEX_INT:
  case NPF_FMT_SPEC_CONV_UNSIGNED_INT:
    switch (length_modifier) {
    case NPF_FMT_SPEC_LEN_MOD_NONE:
    case NPF_FMT_SPEC_LEN_MOD_SHORT:
    case NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE:
    case NPF_FMT_SPEC_LEN_MOD_CHAR:
      return sizeof(unsigned);
    case NPF_FMT_SPEC_LEN_MOD_LONG: // 'l'
      return sizeof(unsigned long);
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
    case NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG:
      return sizeof(unsigned long long);
    case NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX:
      return sizeof(uintmax_t);
    case NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET:
    case NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT:
      return sizeof(size_t);
#    endif
    default:
      return 0;
    }
  case NPF_FMT_SPEC_CONV_POINTER:
    return sizeof(void *);
#    if NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS == 1
  case NPF_FMT_SPEC_CONV_WRITEBACK:
    switch (length_modifier) {
    case NPF_FMT_SPEC_LEN_MOD_NONE:
      return sizeof(int *);
    case NPF_FMT_SPEC_LEN_MOD_SHORT:
      return sizeof(short *);
    case NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE:
      return sizeof(double *);
    case NPF_FMT_SPEC_LEN_MOD_CHAR:
      return sizeof(char *);
    case NPF_FMT_SPEC_LEN_MOD_LONG:
      return sizeof(long *);
#      if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
    case NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG:
      return sizeof(long long *);
    case NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX:
      return sizeof(intmax_t *);
    case NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET:
      return sizeof(ssize_t *);
    case NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT:
      return sizeof(ptrdiff_t *);
#      endif
    default:
      return 0;
    }
#    endif
#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
  case NPF_FMT_SPEC_CONV_FLOAT_DECIMAL:
    if (length_modifier == NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE) {
      return sizeof(long double);
    }
    return sizeof(double);
#    endif
  }
  return 0;
}

static int npf_is_int(struct npf_arg_type *a) {
  if (a->conv_spec != NPF_FMT_SPEC_CONV_SIGNED_INT) {
    return 0;
  }
  switch (a->length_modifier) {
  case NPF_FMT_SPEC_LEN_MOD_NONE:
  case NPF_FMT_SPEC_LEN_MOD_SHORT:
  case NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE:
  case NPF_FMT_SPEC_LEN_MOD_CHAR:
    return 1;
  case NPF_FMT_SPEC_LEN_MOD_LONG:
    return sizeof(int) <= sizeof(long);
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
  case NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG:
    return sizeof(int) <= sizeof(long long);
  case NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX:
    return sizeof(int) <= sizeof(intmax_t);
  case NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET:
    return sizeof(int) <= sizeof(ssize_t);
  case NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT:
    return sizeof(int) <= sizeof(ptrdiff_t);
#    endif
  default:
    return 0;
  }
}

static int npf_format_to_npf_arg_type(NPF_CHAR_TYPE const *const format,
                                      int const nargs,
                                      struct npf_arg_type *const types,
                                      int const accept_new_param) {
  npf_format_spec_t fs;
  int n = 0, used_max = 0;
  NPF_CHAR_TYPE const *cur = format;
  while (*cur) {
    int const fs_len = (*cur != '%') ? 0 : npf_parse_format_spec(cur, &fs);
    if (!fs_len) {
      cur++;
      continue;
    }
    cur += fs_len;
    if (fs.conv_spec == NPF_FMT_SPEC_CONV_PERCENT) {
      continue;
    }
#    if (NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1) && (NANOPRINTF_USE_FMT_SPEC_OPT_STAR == 1)
    if (fs.field_width_opt == NPF_FMT_SPEC_OPT_STAR) {
      if (fs.field_width == 0) {
        fs.field_width = ++n;
      }
      if (fs.field_width > nargs) {
        return -1;
      }
      used_max = npf_max(used_max, fs.field_width);
      struct npf_arg_type *const a = &types[fs.field_width - 1];
      if (accept_new_param && a->conv_spec == NPF_FMT_SPEC_CONV_PERCENT) {
        a->conv_spec = NPF_FMT_SPEC_CONV_SIGNED_INT;
        a->length_modifier = NPF_FMT_SPEC_LEN_MOD_NONE;
      }
      if (!npf_is_int(a)) {
        return -1;
      }
    }
#    endif
#    if (NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1) && (NANOPRINTF_USE_FMT_SPEC_OPT_STAR == 1)
    if (fs.prec_opt == NPF_FMT_SPEC_OPT_STAR) {
      if (fs.prec == 0) {
        fs.prec = ++n;
      }
      if (fs.prec > nargs) {
        return -1;
      }
      used_max = npf_max(used_max, fs.prec);
      struct npf_arg_type *const a = &types[fs.prec - 1];
      if (accept_new_param && a->conv_spec == NPF_FMT_SPEC_CONV_PERCENT) {
        a->conv_spec = NPF_FMT_SPEC_CONV_SIGNED_INT;
        a->length_modifier = NPF_FMT_SPEC_LEN_MOD_NONE;
      }
      if (!npf_is_int(a)) {
        return -1;
      }
    }
#    endif
    if (fs.order == 0) {
      fs.order = ++n;
    }
    if (fs.order > nargs) {
      return -1;
    }
    used_max = npf_max(used_max, fs.order);
    struct npf_arg_type *const a = &types[fs.order - 1];
    if (accept_new_param && a->conv_spec == NPF_FMT_SPEC_CONV_PERCENT) {
      a->conv_spec = fs.conv_spec;
      a->length_modifier = fs.length_modifier;
    }
    if (a->conv_spec != fs.conv_spec) {
      return -1;
    } else if (a->length_modifier != fs.length_modifier &&
               npf_arg_sizeof(fs.conv_spec, fs.length_modifier) != npf_arg_sizeof(a->conv_spec, a->length_modifier)) {
      return -1;
    }
  }
  return used_max;
}

static int npf_verify_and_assign_values(int const args_max,
                                        struct npf_arg_type const *const types,
                                        union npf_arg_value *const values,
                                        va_list args) {
  for (int i = 1; i <= args_max; ++i) {
    int const idx = i - 1;
    switch (types[idx].conv_spec) {
    case NPF_FMT_SPEC_CONV_PERCENT:
      return 0;
    case NPF_FMT_SPEC_CONV_CHAR:
      values[idx].i = va_arg(args, int);
      continue;
    case NPF_FMT_SPEC_CONV_STRING:
      if (types[idx].length_modifier == NPF_FMT_SPEC_LEN_MOD_NONE ||
          types[idx].length_modifier == NPF_FMT_SPEC_LEN_MOD_SHORT) {
        values[idx].hstr = va_arg(args, char *);
      } else if (types[idx].length_modifier == NPF_FMT_SPEC_LEN_MOD_LONG) {
        values[idx].lstr = va_arg(args, wchar_t *);
      } else {
        return 0;
      }
      continue;
    case NPF_FMT_SPEC_CONV_SIGNED_INT:
      switch (types[idx].length_modifier) {
      case NPF_FMT_SPEC_LEN_MOD_NONE:
      case NPF_FMT_SPEC_LEN_MOD_SHORT:
      case NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE:
      case NPF_FMT_SPEC_LEN_MOD_CHAR:
        values[idx].i = va_arg(args, int);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LONG:
        values[idx].l = va_arg(args, long);
        continue;
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
      case NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG:
        values[idx].ll = va_arg(args, long long);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX:
        values[idx].imx = va_arg(args, intmax_t);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET:
        values[idx].ssz = va_arg(args, ssize_t);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT:
        values[idx].ptrdiff = va_arg(args, ptrdiff_t);
        continue;
#    endif
      default:
        return 0;
      }
#    if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
    case NPF_FMT_SPEC_CONV_BINARY:
#    endif
    case NPF_FMT_SPEC_CONV_OCTAL:
    case NPF_FMT_SPEC_CONV_HEX_INT:
    case NPF_FMT_SPEC_CONV_UNSIGNED_INT:
      switch (types[idx].length_modifier) {
      case NPF_FMT_SPEC_LEN_MOD_NONE:
      case NPF_FMT_SPEC_LEN_MOD_SHORT:
      case NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE:
      case NPF_FMT_SPEC_LEN_MOD_CHAR:
        values[idx].u = va_arg(args, unsigned);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LONG: // 'l'
        values[idx].ul = va_arg(args, unsigned long);
        continue;
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
      case NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG:
        values[idx].ull = va_arg(args, unsigned long long);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX:
        values[idx].uimx = va_arg(args, uintmax_t);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET:
      case NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT:
        values[idx].sz = va_arg(args, size_t);
        continue;
#    endif
      default:
        return 0;
      }
    case NPF_FMT_SPEC_CONV_POINTER:
      values[idx].ptr = va_arg(args, void *);
      continue;
#    if NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS == 1
    case NPF_FMT_SPEC_CONV_WRITEBACK:
      switch (types[idx].length_modifier) {
      case NPF_FMT_SPEC_LEN_MOD_NONE:
        values[idx].ip = va_arg(args, int *);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_SHORT:
        values[idx].sp = va_arg(args, short *);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE:
        values[idx].dp = va_arg(args, double *);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_CHAR:
        values[idx].cp = va_arg(args, char *);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LONG:
        values[idx].lp = va_arg(args, long *);
        continue;
#      if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
      case NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG:
        values[idx].llp = va_arg(args, long long *);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX:
        values[idx].imxp = va_arg(args, intmax_t *);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET:
        values[idx].sszp = va_arg(args, ssize_t *);
        continue;
      case NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT:
        values[idx].ptrdiffp = va_arg(args, ptrdiff_t *);
        continue;
#      endif
      default:
        return 0;
      }
#    endif
#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
    case NPF_FMT_SPEC_CONV_FLOAT_DECIMAL:
      if (types[idx].length_modifier == NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE) {
        values[idx].ld = va_arg(args, long double);
        continue;
      }
      values[idx].d = va_arg(args, double);
      continue;
#    endif
    }
  }
  return 1;
}

int npf_verify_format(NPF_CHAR_TYPE const *reference, NPF_CHAR_TYPE const *format) {
  enum {
    max_args = 64,
  };
  struct npf_arg_type args[max_args] = {0};
  int const used_max = npf_format_to_npf_arg_type(reference, max_args, args, 1);
  if (used_max == -1) {
    return 0;
  }
  // check for unused arguments
  for (int i = 1; i <= used_max; ++i) {
    if (args[i - 1].conv_spec == NPF_FMT_SPEC_CONV_PERCENT) {
      return 0;
    }
  }
  if (format == NULL || reference == format) {
    return 1;
  }
  int const n = npf_format_to_npf_arg_type(format, used_max, args, 0);
  if (n == -1) {
    return 0;
  }
  return 1;
}

void npf_bufputc(int c, void *ctx) {
  npf_bufputc_ctx_t *bpc = (npf_bufputc_ctx_t *)ctx;
  if (bpc->cur < bpc->len) {
    bpc->dst[bpc->cur++] = (NPF_CHAR_TYPE)c;
  }
}

void npf_bufputc_nop(int c, void *ctx) {
  (void)c;
  (void)ctx;
}

typedef struct npf_cnt_putc_ctx {
  npf_putc pc;
  void *ctx;
  int n;
} npf_cnt_putc_ctx_t;

static void npf_putc_cnt(int c, void *ctx) {
  npf_cnt_putc_ctx_t *pc_cnt = (npf_cnt_putc_ctx_t *)ctx;
  ++pc_cnt->n;
  pc_cnt->pc(c, pc_cnt->ctx); // sibling-call optimization
}

struct ovutf_context {
  npf_putc pc;
  void *pc_ctx;
};

static enum ov_codepoint_fn_result write_codepoint(int_fast32_t codepoint, void *ctx) {
  struct ovutf_context *const c = ctx;
  if (sizeof(NPF_CHAR_TYPE) != sizeof(char)) {
    if (sizeof(wchar_t) == 2 && codepoint > 0xffff) {
      c->pc((int)((codepoint - 0x10000) / 0x400 + 0xd800), c->pc_ctx);
      c->pc((int)((codepoint - 0x10000) % 0x400 + 0xdc00), c->pc_ctx);
      return ov_codepoint_fn_result_continue;
    }
    c->pc((int)codepoint, c->pc_ctx);
    return ov_codepoint_fn_result_continue;
  }
  if (codepoint < 0x80) {
    c->pc((int)(codepoint & 0x7f), c->pc_ctx);
    return ov_codepoint_fn_result_continue;
  }
  if (codepoint < 0x800) {
    c->pc((int)(0xc0 | ((codepoint >> 6) & 0x1f)), c->pc_ctx);
    c->pc((int)(0x80 | (codepoint & 0x3f)), c->pc_ctx);
    return ov_codepoint_fn_result_continue;
  }
  if (codepoint < 0x10000) {
    c->pc((int)(0xe0 | ((codepoint >> 12) & 0x0f)), c->pc_ctx);
    c->pc((int)(0x80 | ((codepoint >> 6) & 0x3f)), c->pc_ctx);
    c->pc((int)(0x80 | (codepoint & 0x3f)), c->pc_ctx);
    return ov_codepoint_fn_result_continue;
  }
  c->pc((int)(0xf0 | ((codepoint >> 18) & 0x07)), c->pc_ctx);
  c->pc((int)(0x80 | ((codepoint >> 12) & 0x3f)), c->pc_ctx);
  c->pc((int)(0x80 | ((codepoint >> 6) & 0x3f)), c->pc_ctx);
  c->pc((int)(0x80 | (codepoint & 0x3f)), c->pc_ctx);
  return ov_codepoint_fn_result_continue;
}

#    define NPF_PUTC(VAL)                                                                                              \
      do {                                                                                                             \
        npf_putc_cnt((int)(VAL), &pc_cnt);                                                                             \
      } while (0)

#    define NPF_EXTRACT(MOD, CAST_TO, FROM)                                                                            \
    case NPF_FMT_SPEC_LEN_MOD_##MOD:                                                                                   \
      val = (CAST_TO)(FROM);                                                                                           \
      break

#    define NPF_WRITEBACK(MOD, TYPE)                                                                                   \
    case NPF_FMT_SPEC_LEN_MOD_##MOD:                                                                                   \
      *(va_arg(args, TYPE *)) = (TYPE)pc_cnt.n;                                                                        \
      break

int npf_vpprintf(npf_putc pc, void *pc_ctx, NPF_CHAR_TYPE const *reference, NPF_CHAR_TYPE const *format, va_list args) {
  enum {
    max_args = 64,
  };
  struct npf_arg_type arg_types[max_args] = {0};
  union npf_arg_value arg_values[max_args] = {0};
  int used_args = npf_format_to_npf_arg_type(reference ? reference : format, max_args, arg_types, 1);
  if (used_args == -1) {
    return 0;
  }
  if (reference != format) {
    used_args = npf_format_to_npf_arg_type(format, used_args, arg_types, 0);
    if (used_args == -1) {
      return 0;
    }
  }
  if (npf_verify_and_assign_values(used_args, arg_types, arg_values, args) == 0) {
    return 0;
  }

  npf_format_spec_t fs;
  NPF_CHAR_TYPE const *cur = format;
  npf_cnt_putc_ctx_t pc_cnt;
  pc_cnt.pc = pc;
  pc_cnt.ctx = pc_ctx;
  pc_cnt.n = 0;

  int arg_index = 0;
  while (*cur) {
    int const fs_len = (*cur != '%') ? 0 : npf_parse_format_spec(cur, &fs);
    if (!fs_len) {
      NPF_PUTC(*cur++);
      continue;
    }
    cur += fs_len;

    // Extract star-args immediately
#    if (NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1) && (NANOPRINTF_USE_FMT_SPEC_OPT_STAR == 1)
    if (fs.field_width_opt == NPF_FMT_SPEC_OPT_STAR) {
      fs.field_width_opt = NPF_FMT_SPEC_OPT_LITERAL;
      fs.field_width = arg_values[(fs.field_width == 0 ? ++arg_index : fs.field_width) - 1].i;
      if (fs.field_width < 0) {
        fs.field_width = -fs.field_width;
        fs.left_justified = 1;
      }
    }
#    endif
#    if (NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1) && (NANOPRINTF_USE_FMT_SPEC_OPT_STAR == 1)
    if (fs.prec_opt == NPF_FMT_SPEC_OPT_STAR) {
      fs.prec_opt = NPF_FMT_SPEC_OPT_NONE;
      fs.prec = arg_values[(fs.prec == 0 ? ++arg_index : fs.prec) - 1].i;
      if (fs.prec >= 0) {
        fs.prec_opt = NPF_FMT_SPEC_OPT_LITERAL;
      }
    }
#    endif
    if (fs.conv_spec != NPF_FMT_SPEC_CONV_PERCENT && fs.order == 0) {
      fs.order = ++arg_index;
    }

    union {
      NPF_CHAR_TYPE cbuf_mem[32];
      npf_uint_t binval;
    } u;
    NPF_CHAR_TYPE *cbuf = u.cbuf_mem, sign_c = 0;
    int cbuf_len = 0, cbuf_origlen = 0, need_0x = 0;
#    if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    int field_pad = 0;
    NPF_CHAR_TYPE pad_c = 0;
#    endif
#    if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
    int prec_pad = 0;
#      if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    int zero = 0;
#      endif
#    endif
#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
    int frac_chars = 0, inf_or_nan = 0;
#    endif

    // Extract and convert the argument to string, point cbuf at the text.
    switch (fs.conv_spec) {
    case NPF_FMT_SPEC_CONV_PERCENT:
      *cbuf = '%';
      ++cbuf_len;
      break;

    case NPF_FMT_SPEC_CONV_CHAR:
      *cbuf = (NPF_CHAR_TYPE)(arg_values[fs.order - 1].i);
      ++cbuf_len;
      break;

    case NPF_FMT_SPEC_CONV_STRING: {
      if (fs.length_modifier == NPF_FMT_SPEC_LEN_MOD_NONE || fs.length_modifier == NPF_FMT_SPEC_LEN_MOD_SHORT) {
        cbuf = (void *)arg_values[fs.order - 1].hstr;
        for (char const *s = (void *)cbuf; *s; ++s, ++cbuf_len)
          ; // strlen
        if (sizeof(NPF_CHAR_TYPE) != sizeof(char)) {
          cbuf_origlen = cbuf_len;
          if (cbuf_len) {
            cbuf_len = (int)ov_utf8_to_wchar_len((void *)cbuf, (size_t)cbuf_len);
            if (!cbuf_len) {
              return 0;
            }
          }
        }
      } else if (fs.length_modifier == NPF_FMT_SPEC_LEN_MOD_LONG) {
        cbuf = (void *)arg_values[fs.order - 1].lstr;
        for (wchar_t const *s = (void *)cbuf; *s; ++s, ++cbuf_len)
          ; // wcslen
        if (sizeof(NPF_CHAR_TYPE) != sizeof(wchar_t)) {
          cbuf_origlen = cbuf_len;
          if (cbuf_len) {
            cbuf_len = (int)ov_wchar_to_utf8_len((void *)cbuf, (size_t)cbuf_len);
            if (!cbuf_len) {
              return 0;
            }
          }
        }
      } else {
        return 0;
      }
#    if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
      if (fs.prec_opt == NPF_FMT_SPEC_OPT_LITERAL) {
        cbuf_len = npf_min(fs.prec, cbuf_len); // prec truncates strings
      }
#    endif
    } break;

    case NPF_FMT_SPEC_CONV_SIGNED_INT: {
      npf_int_t val = 0;
      switch (fs.length_modifier) {
        NPF_EXTRACT(NONE, int, arg_values[fs.order - 1].i);
        NPF_EXTRACT(SHORT, short, arg_values[fs.order - 1].i);
        NPF_EXTRACT(LONG_DOUBLE, int, arg_values[fs.order - 1].i);
        NPF_EXTRACT(CHAR, char, arg_values[fs.order - 1].i);
        NPF_EXTRACT(LONG, long, arg_values[fs.order - 1].l);
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
        NPF_EXTRACT(LARGE_LONG_LONG, long long, arg_values[fs.order - 1].ll);
        NPF_EXTRACT(LARGE_INTMAX, intmax_t, arg_values[fs.order - 1].imx);
        NPF_EXTRACT(LARGE_SIZET, ssize_t, arg_values[fs.order - 1].ssz);
        NPF_EXTRACT(LARGE_PTRDIFFT, ptrdiff_t, arg_values[fs.order - 1].ptrdiff);
#    endif
      default:
        break;
      }

      sign_c = (val < 0) ? '-' : fs.prepend;

#    if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
#      if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
      zero = !val;
#      endif
      // special case, if prec and value are 0, skip
      if (!val && (fs.prec_opt == NPF_FMT_SPEC_OPT_LITERAL) && !fs.prec) {
        cbuf_len = 0;
      } else
#    endif
      {
        cbuf_len = npf_itoa_rev(cbuf, val);
      }
    } break;

#    if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
    case NPF_FMT_SPEC_CONV_BINARY:
#    endif
    case NPF_FMT_SPEC_CONV_OCTAL:
    case NPF_FMT_SPEC_CONV_HEX_INT:
    case NPF_FMT_SPEC_CONV_UNSIGNED_INT: {
      npf_uint_t val = 0;

      switch (fs.length_modifier) {
        NPF_EXTRACT(NONE, unsigned, arg_values[fs.order - 1].u);
        NPF_EXTRACT(SHORT, unsigned short, arg_values[fs.order - 1].u);
        NPF_EXTRACT(LONG_DOUBLE, unsigned, arg_values[fs.order - 1].u);
        NPF_EXTRACT(CHAR, unsigned char, arg_values[fs.order - 1].u);
        NPF_EXTRACT(LONG, unsigned long, arg_values[fs.order - 1].ul);
#    if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
        NPF_EXTRACT(LARGE_LONG_LONG, unsigned long long, arg_values[fs.order - 1].ull);
        NPF_EXTRACT(LARGE_INTMAX, uintmax_t, arg_values[fs.order - 1].uimx);
        NPF_EXTRACT(LARGE_SIZET, size_t, arg_values[fs.order - 1].sz);
        NPF_EXTRACT(LARGE_PTRDIFFT, size_t, arg_values[fs.order - 1].sz);
#    endif
      default:
        break;
      }

#    if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
#      if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
      zero = !val;
#      endif
      if (!val && (fs.prec_opt == NPF_FMT_SPEC_OPT_LITERAL) && !fs.prec) {
        // Zero value and explicitly-requested zero precision means "print nothing".
        if ((fs.conv_spec == NPF_FMT_SPEC_CONV_OCTAL) && fs.alt_form) {
          fs.prec = 1; // octal special case, print a single '0'
        }
      } else
#    endif
#    if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
          if (fs.conv_spec == NPF_FMT_SPEC_CONV_BINARY) {
        cbuf_len = npf_bin_len(val);
        u.binval = val;
      } else
#    endif
      {
        unsigned const base =
            (fs.conv_spec == NPF_FMT_SPEC_CONV_OCTAL) ? 8u : ((fs.conv_spec == NPF_FMT_SPEC_CONV_HEX_INT) ? 16u : 10u);
        cbuf_len = npf_utoa_rev(cbuf, val, base, (unsigned)fs.case_adjust);
      }

      if (val && fs.alt_form && (fs.conv_spec == NPF_FMT_SPEC_CONV_OCTAL)) {
        cbuf[cbuf_len++] = '0'; // OK to add leading octal '0' immediately.
      }

      if (val && fs.alt_form) { // 0x or 0b but can't write it yet.
        if (fs.conv_spec == NPF_FMT_SPEC_CONV_HEX_INT) {
          need_0x = 'X';
        }
#    if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
        else if (fs.conv_spec == NPF_FMT_SPEC_CONV_BINARY) {
          need_0x = 'B';
        }
#    endif
        if (need_0x) {
          need_0x += fs.case_adjust;
        }
      }
    } break;

    case NPF_FMT_SPEC_CONV_POINTER: {
      cbuf_len = npf_utoa_rev(cbuf, (npf_uint_t)(uintptr_t)(arg_values[fs.order - 1].ptr), 16, 'a' - 'A');
      need_0x = 'x';
    } break;

#    if NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS == 1
    case NPF_FMT_SPEC_CONV_WRITEBACK:
      switch (fs.length_modifier) {
        NPF_WRITEBACK(NONE, int);
        NPF_WRITEBACK(SHORT, short);
        NPF_WRITEBACK(LONG, long);
        NPF_WRITEBACK(LONG_DOUBLE, double);
        NPF_WRITEBACK(CHAR, char);
#      if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
        NPF_WRITEBACK(LARGE_LONG_LONG, long long);
        NPF_WRITEBACK(LARGE_INTMAX, intmax_t);
        NPF_WRITEBACK(LARGE_SIZET, size_t);
        NPF_WRITEBACK(LARGE_PTRDIFFT, ptrdiff_t);
#      endif
      default:
        break;
      }
      break;
#    endif

#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
    case NPF_FMT_SPEC_CONV_FLOAT_DECIMAL: {
      float val;
      if (fs.length_modifier == NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE) {
        val = (float)(arg_values[fs.order - 1].ld);
      } else {
        val = (float)(arg_values[fs.order - 1].d);
      }

      sign_c = (val < 0.f) ? '-' : fs.prepend;
#      if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
      zero = (val == 0.f);
#      endif
      cbuf_len = npf_ftoa_rev(cbuf, val, fs.case_adjust, &frac_chars);

      if (cbuf_len < 0) {
        cbuf_len = -cbuf_len;
        inf_or_nan = 1;
      } else {
        int const prec_adj = npf_max(0, frac_chars - fs.prec);
        cbuf += prec_adj;
        cbuf_len -= prec_adj;
      }
    } break;
#    endif
    default:
      break;
    }

#    if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    // Compute the field width pad character
    if (fs.field_width_opt == NPF_FMT_SPEC_OPT_LITERAL) {
      if (fs.leading_zero_pad) { // '0' flag is only legal with numeric types
        if ((fs.conv_spec != NPF_FMT_SPEC_CONV_STRING) && (fs.conv_spec != NPF_FMT_SPEC_CONV_CHAR) &&
            (fs.conv_spec != NPF_FMT_SPEC_CONV_PERCENT)) {
#      if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
          if ((fs.prec_opt == NPF_FMT_SPEC_OPT_LITERAL) && !fs.prec && zero) {
            pad_c = ' ';
          } else
#      endif
          {
            pad_c = '0';
          }
        }
      } else {
        pad_c = ' ';
      }
    }
#    endif

    // Compute the number of bytes to truncate or '0'-pad.
    if (fs.conv_spec != NPF_FMT_SPEC_CONV_STRING) {
#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
      if (!inf_or_nan) { // float precision is after the decimal point
        int const prec_start = (fs.conv_spec == NPF_FMT_SPEC_CONV_FLOAT_DECIMAL) ? frac_chars : cbuf_len;
        prec_pad = npf_max(0, fs.prec - prec_start);
      }
#    elif NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
      prec_pad = npf_max(0, fs.prec - cbuf_len);
#    endif
    }

#    if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    // Given the full converted length, how many pad bytes?
    field_pad = fs.field_width - cbuf_len - !!sign_c;
    if (need_0x) {
      field_pad -= 2;
    }

#      if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
    if ((fs.conv_spec == NPF_FMT_SPEC_CONV_FLOAT_DECIMAL) && !fs.prec && !fs.alt_form) {
      ++field_pad; // 0-pad, no decimal point.
    }
#      endif
#      if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
    field_pad -= prec_pad;
#      endif
    field_pad = npf_max(0, field_pad);
#    endif // NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS

#    if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    // Apply right-justified field width if requested
    if (!fs.left_justified && pad_c) { // If leading zeros pad, sign goes first.
      if (pad_c == '0') {
        if (sign_c) {
          NPF_PUTC(sign_c);
          sign_c = 0;
        }
        // Pad byte is '0', write '0x' before '0' pad chars.
        if (need_0x) {
          NPF_PUTC('0');
          NPF_PUTC(need_0x);
        }
      }
      while (field_pad-- > 0) {
        NPF_PUTC(pad_c);
      }
      // Pad byte is ' ', write '0x' after ' ' pad chars but before number.
      if ((pad_c != '0') && need_0x) {
        NPF_PUTC('0');
        NPF_PUTC(need_0x);
      }
    } else
#    endif
    {
      if (need_0x) {
        NPF_PUTC('0');
        NPF_PUTC(need_0x);
      }
    } // no pad, '0x' requested.

    // Write the converted payload
    if (fs.conv_spec == NPF_FMT_SPEC_CONV_STRING) {
      if (fs.length_modifier == NPF_FMT_SPEC_LEN_MOD_NONE || fs.length_modifier == NPF_FMT_SPEC_LEN_MOD_SHORT) {
        if (sizeof(NPF_CHAR_TYPE) != sizeof(char) && cbuf_origlen) {
          if (!ov_utf8_to_codepoint(write_codepoint,
                                    &(struct ovutf_context){.pc = npf_putc_cnt, .pc_ctx = &pc_cnt},
                                    (void *)cbuf,
                                    (size_t)cbuf_origlen)) {
            return 0;
          }
        } else {
          for (int i = 0; i < cbuf_len; ++i) {
            NPF_PUTC(cbuf[i]);
          }
        }
      } else if (fs.length_modifier == NPF_FMT_SPEC_LEN_MOD_LONG) {
        if (sizeof(NPF_CHAR_TYPE) != sizeof(wchar_t) && cbuf_origlen) {
          if (!ov_wchar_to_codepoint(write_codepoint,
                                     &(struct ovutf_context){.pc = npf_putc_cnt, .pc_ctx = &pc_cnt},
                                     (void *)cbuf,
                                     (size_t)cbuf_origlen)) {
            return 0;
          }
        } else {
          for (int i = 0; i < cbuf_len; ++i) {
            NPF_PUTC(cbuf[i]);
          }
        }
      }
    } else {
      if (sign_c) {
        NPF_PUTC(sign_c);
      }
#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
      if (fs.conv_spec != NPF_FMT_SPEC_CONV_FLOAT_DECIMAL) {
#    endif

#    if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
        while (prec_pad-- > 0) {
          NPF_PUTC('0');
        } // int precision leads.
#    endif

#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
      } else {
        // if 0 precision, skip the fractional part and '.'
        // if 0 prec + alternative form, keep the '.'
        if (!fs.prec && !fs.alt_form) {
          ++cbuf;
          --cbuf_len;
        }
      }
#    endif

#    if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
      if (fs.conv_spec == NPF_FMT_SPEC_CONV_BINARY) {
        while (cbuf_len) {
          NPF_PUTC('0' + ((u.binval >> --cbuf_len) & 1));
        }
      } else
#    endif
      {
        while (cbuf_len-- > 0) {
          NPF_PUTC(cbuf[cbuf_len]);
        }
      } // payload is reversed

#    if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
      // real precision comes after the number.
      if ((fs.conv_spec == NPF_FMT_SPEC_CONV_FLOAT_DECIMAL) && !inf_or_nan) {
        while (prec_pad-- > 0) {
          NPF_PUTC('0');
        }
      }
#    endif
    }

#    if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    if (fs.left_justified && pad_c) { // Apply left-justified field width
      while (field_pad-- > 0) {
        NPF_PUTC(pad_c);
      }
    }
#    endif
  }

  return pc_cnt.n;
}

#    undef NPF_PUTC
#    undef NPF_EXTRACT
#    undef NPF_WRITEBACK

int npf_pprintf(npf_putc pc, void *pc_ctx, NPF_CHAR_TYPE const *reference, NPF_CHAR_TYPE const *format, ...) {
  va_list val;
  va_start(val, format);
  int const rv = npf_vpprintf(pc, pc_ctx, reference, format, val);
  va_end(val);
  return rv;
}

int npf_snprintf(
    NPF_CHAR_TYPE *buffer, size_t bufsz, NPF_CHAR_TYPE const *reference, const NPF_CHAR_TYPE *format, ...) {
  va_list val;
  va_start(val, format);
  int const rv = npf_vsnprintf(buffer, bufsz, reference, format, val);
  va_end(val);
  return rv;
}

int npf_vsnprintf(
    NPF_CHAR_TYPE *buffer, size_t bufsz, NPF_CHAR_TYPE const *reference, NPF_CHAR_TYPE const *format, va_list vlist) {
  npf_bufputc_ctx_t bufputc_ctx;
  bufputc_ctx.dst = buffer;
  bufputc_ctx.len = bufsz;
  bufputc_ctx.cur = 0;

  npf_putc const pc = buffer ? npf_bufputc : npf_bufputc_nop;
  int const n = npf_vpprintf(pc, &bufputc_ctx, reference, format, vlist);
  pc('\0', &bufputc_ctx);

#    ifdef NANOPRINTF_SNPRINTF_SAFE_EMPTY_STRING_ON_OVERFLOW
  if (bufsz && (n >= (int)bufsz)) {
    buffer[0] = '\0';
  }
#    elif defined(NANOPRINTF_SNPRINTF_SAFE_TRIM_STRING_ON_OVERFLOW)
  if (bufsz && (n >= (int)bufsz)) {
    buffer[bufsz - 1] = '\0';
  }
#    endif

  return n;
}

#    if NANOPRINTF_HAVE_GCC_WARNING_PRAGMAS
#      pragma GCC diagnostic pop
#    endif

#    ifdef _MSC_VER
#      pragma warning(pop)
#    endif

#  endif // NANOPRINTF_IMPLEMENTATION_INCLUDED
#endif   // NANOPRINTF_IMPLEMENTATION

/*
  nanoprintf is dual-licensed under both the "Unlicense" and the
  "Zero-Clause BSD" (0BSD) licenses. The intent of this dual-licensing
  structure is to make nanoprintf as consumable as possible in as many
  environments / countries / companies as possible without any
  encumberances.

  The text of the two licenses follows below:

  ============================== UNLICENSE ==============================

  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org>

  ================================ 0BSD =================================

  Copyright (C) 2019- by Charles Nicholson <charles.nicholson+nanoprintf@gmail.com>

  Permission to use, copy, modify, and/or distribute this software for
  any purpose with or without fee is hereby granted.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
