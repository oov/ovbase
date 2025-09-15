#include <ovtest.h>

#include <ovarray.h>
#include <ovprintf_ex.h>

#ifndef CHAR_TYPE
#  error "CHAR_TYPE must be defined before including this file"
#endif

#ifndef PREFIX
#  error "PREFIX must be defined before including this file"
#endif

#ifndef POSTFIX
#  error "POSTFIX must be defined before including this file"
#endif

#ifndef STR_LITERAL
#  error "STR_LITERAL must be defined before including this file"
#endif

#ifndef STR_CMP
#  error "STR_CMP must be defined before including this file"
#endif

#ifndef PRINTF_STR_SPEC
#  error "PRINTF_STR_SPEC must be defined before including this file"
#endif

#ifndef FUNCNAME
#  define FUNCNAME_3(prefix, funcname, postfix) prefix##funcname##postfix
#  define FUNCNAME_2(prefix, funcname, postfix) FUNCNAME_3(prefix, funcname, postfix)
#  define FUNCNAME(funcname) FUNCNAME_2(PREFIX, funcname, POSTFIX)
#endif

#ifndef TESTNAME
#  define TESTNAME_3(prefix, funcname, postfix) prefix##funcname##postfix
#  define TESTNAME_2(prefix, funcname, postfix) TESTNAME_3(prefix, funcname, postfix)
#  define TESTNAME(funcname) TESTNAME_2(test_, funcname, POSTFIX)
#endif

static void TESTNAME(ov_sprintf)(void) {
  CHAR_TYPE *str = NULL;

  // Basic formatting test
  TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, STR_LITERAL("Hello, " PRINTF_STR_SPEC "!"), STR_LITERAL("world")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Hello, world!")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);
  OV_ARRAY_DESTROY(&str);

  // Number formatting test
  TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, STR_LITERAL("Number: %d"), 42));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Number: 42")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);

  // Reuse buffer test
  TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, STR_LITERAL("First")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("First")) == 0);

  TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, STR_LITERAL("Second message is longer")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Second message is longer")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);
}

static bool TESTNAME(vsprintf_helper)(CHAR_TYPE **dest, CHAR_TYPE const *fmt, ...) {
  va_list valist;
  va_start(valist, fmt);
  bool result = FUNCNAME(vsprintf)(dest, NULL, fmt, valist);
  va_end(valist);
  return result;
}

static void TESTNAME(ov_vsprintf)(void) {
  CHAR_TYPE *str = NULL;

  TEST_CHECK(TESTNAME(vsprintf_helper)(&str, STR_LITERAL("Value: %d"), 123));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Value: 123")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);
}

static void TESTNAME(ov_sprintf_append)(void) {
  CHAR_TYPE *str = NULL;

  // Start with initial string
  TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, STR_LITERAL("Hello")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Hello")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);

  // Append to existing string
  TEST_CHECK(FUNCNAME(sprintf_append)(&str, NULL, STR_LITERAL(", " PRINTF_STR_SPEC "!"), STR_LITERAL("world")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Hello, world!")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);

  // Append more
  TEST_CHECK(FUNCNAME(sprintf_append)(&str, NULL, STR_LITERAL(" Number: %d"), 42));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Hello, world! Number: 42")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);

  // Append to NULL string (should work like normal sprintf)
  str = NULL;
  TEST_CHECK(FUNCNAME(sprintf_append)(&str, NULL, STR_LITERAL("First")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("First")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);
  OV_ARRAY_DESTROY(&str);
}
