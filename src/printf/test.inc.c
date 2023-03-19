#include "ovtest.h"
#include <inttypes.h>

enum {
  bufsize = 128,
};

static void test1(void) {
  CHAR_TYPE buf[bufsize];
  TEST_CHECK(ov_snprintf(NULL, 0, TT("hello%04d%s."), 20, TT("world")) == 15);
  TEST_CHECK(ov_snprintf(buf, 10, TT("hello%04d%s."), 20, TT("world")) == 15);
  TEST_CHECK(ov_snprintf(buf, bufsize, TT("hello%04d%s."), 20, TT("world")) == 15);
  TEST_CHECK(STRCMP(TT("hello0020world."), buf) == 0);
}

static void test_printf_verify_format(void) {
  TEST_CHECK(ov_printf_verify_format(TT("%d%s%d"), TT("hello %d world %s.")) != 0);
  TEST_CHECK(ov_printf_verify_format(TT("%d%s%d"), TT("hello %s world %s.")) == 0);
  TEST_CHECK(ov_printf_verify_format(TT("%0.*f%s%d"), TT("hello %f world %s.")) == 0);
  TEST_CHECK(ov_printf_verify_format(TT("%0.*f%s%d"), TT("hello %0.*f world %s.")) != 0);
}

static void test_printf_order_extension(void) {
  CHAR_TYPE buf[bufsize];
  TEST_CHECK(ov_printf_verify_format(TT("%d%s%d"), TT("hello %2$s world %1$d.")) != 0);
  TEST_CHECK(ov_printf_verify_format(TT("hello %2$s world %3$d."), NULL) == 0);
  TEST_CHECK(ov_printf_verify_format(TT("%d%s%d"), TT("hello %2$s world %3$d.")) != 0);
  TEST_CHECK(ov_snprintf(buf, bufsize, TT("%3$s%2$04d%1$s."), TT("world"), 20, TT("hello")) == 15);
  TEST_CHECK(STRCMP(TT("hello0020world."), buf) == 0);
}

TEST_LIST = {
    {"test1", test1},
    {"test_printf_verify_format", test_printf_verify_format},
    {"test_printf_order_extension", test_printf_order_extension},
    {NULL, NULL},
};
