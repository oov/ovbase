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
  TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, NULL, STR_LITERAL("Hello, " PRINTF_STR_SPEC "!"), STR_LITERAL("world")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Hello, world!")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);
  OV_ARRAY_DESTROY(&str);

  // Number formatting test
  TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, NULL, STR_LITERAL("Number: %d"), 42));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Number: 42")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 10);
  OV_ARRAY_DESTROY(&str);

  // Reuse buffer test
  TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, NULL, STR_LITERAL("First")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("First")) == 0);

  TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, NULL, STR_LITERAL("Second message is longer")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Second message is longer")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);
}

static bool TESTNAME(vsprintf_helper)(CHAR_TYPE **dest, CHAR_TYPE const *fmt, ...) {
  va_list valist;
  va_start(valist, fmt);
  bool result = FUNCNAME(vsprintf)(dest, NULL, NULL, fmt, valist);
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
  TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, NULL, STR_LITERAL("Hello")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Hello")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);

  // Append to existing string
  TEST_CHECK(FUNCNAME(sprintf_append)(&str, NULL, NULL, STR_LITERAL(", " PRINTF_STR_SPEC "!"), STR_LITERAL("world")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Hello, world!")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 13);

  // Append more
  TEST_CHECK(FUNCNAME(sprintf_append)(&str, NULL, NULL, STR_LITERAL(" Number: %d"), 42));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("Hello, world! Number: 42")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 24);
  OV_ARRAY_DESTROY(&str);

  // Append to NULL string (should work like normal sprintf)
  str = NULL;
  TEST_CHECK(FUNCNAME(sprintf_append)(&str, NULL, NULL, STR_LITERAL("First")));
  TEST_CHECK(STR_CMP(str, STR_LITERAL("First")) == 0);
  TEST_CHECK(OV_ARRAY_LENGTH(str) == 5);
  OV_ARRAY_DESTROY(&str);
}

static void TESTNAME(ov_sprintf_format_mismatch)(void) {
  CHAR_TYPE *str = NULL;

  // reference and format have different argument types: %d vs %s
  // This should fail with a proper error, NOT ov_error_generic_out_of_memory
  {
    struct ov_error err = {0};
    TEST_CHECK(!FUNCNAME(sprintf)(&str, &err, STR_LITERAL("%1$d"), STR_LITERAL("%1$s"), STR_LITERAL("hello")));
    TEST_CHECK(str == NULL);
    // Verify it's NOT out_of_memory error
    TEST_CHECK(!ov_error_is(&err, ov_error_type_generic, ov_error_generic_out_of_memory));
    OV_ERROR_DESTROY(&err);
  }

  // format references more arguments than reference provides
  {
    struct ov_error err = {0};
    TEST_CHECK(!FUNCNAME(sprintf)(&str, &err, STR_LITERAL("%1$d"), STR_LITERAL("%1$d %2$d"), 42, 100));
    TEST_CHECK(str == NULL);
    // Verify it's NOT out_of_memory error
    TEST_CHECK(!ov_error_is(&err, ov_error_type_generic, ov_error_generic_out_of_memory));
    OV_ERROR_DESTROY(&err);
  }

  // append version should also return format mismatch error
  {
    struct ov_error err = {0};
    TEST_CHECK(FUNCNAME(sprintf)(&str, NULL, NULL, STR_LITERAL("prefix")));
    TEST_CHECK(!FUNCNAME(sprintf_append)(&str, &err, STR_LITERAL("%1$d"), STR_LITERAL("%1$s"), STR_LITERAL("hello")));
    // On format mismatch during append, existing content should be preserved
    TEST_CHECK(STR_CMP(str, STR_LITERAL("prefix")) == 0);
    // Verify it's NOT out_of_memory error
    TEST_CHECK(!ov_error_is(&err, ov_error_type_generic, ov_error_generic_out_of_memory));
    OV_ERROR_DESTROY(&err);
    OV_ARRAY_DESTROY(&str);
  }
}
