#include <inttypes.h>
#include <ovtest.h>

enum {
  bufsize = 128,
};

static void test1(void) {
  CHAR_TYPE buf[bufsize];
  TEST_CHECK(ov_snprintf(NULL, 0, NULL, TT("hello%04d%") STRFMT TT("."), 20, TT("world")) == 15);
  TEST_CHECK(ov_snprintf(buf, 10, NULL, TT("hello%04d%") STRFMT TT("."), 20, TT("world")) == 15);
  TEST_CHECK(ov_snprintf(buf, bufsize, NULL, TT("hello%04d%") STRFMT TT("."), 20, TT("world")) == 15);
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
  TEST_CHECK(
      ov_snprintf(buf, bufsize, NULL, TT("%3$") STRFMT TT("%2$04d%1$") STRFMT TT("."), TT("world"), 20, TT("hello")) ==
      15);
  TEST_CHECK(STRCMP(TT("hello0020world."), buf) == 0);
  TEST_CHECK(ov_snprintf(buf, bufsize, NULL, TT("%3$") STRFMT TT("%2$04d."), TT("world"), 20, TT("hello")) == 0);
  TEST_CHECK(ov_snprintf(buf,
                         bufsize,
                         TT("%3$") STRFMT TT("%2$04d%1$") STRFMT TT("."),
                         TT("%3$") STRFMT TT("%2$04d."),
                         TT("world"),
                         20,
                         TT("hello")) == 10);
  TEST_CHECK(STRCMP(TT("hello0020."), buf) == 0);
}

static void test_string_ascii(void) {
  CHAR_TYPE buf[bufsize];
  TEST_CHECK(ov_snprintf(buf, bufsize, NULL, TT("%hs%ls"), "hello", L"world") > 0);
  TEST_CHECK(STRCMP(buf, TT("helloworld")) == 0);
  TEST_MSG(sizeof(CHAR_TYPE) == 1 ? "want %s, got %s" : "want %ls, got %ls", TT("helloworld"), buf);
}

static void test_string_s(void) {
  CHAR_TYPE buf[bufsize];
  TEST_CHECK(ov_snprintf(buf, bufsize, NULL, TT("%s"), "ＡＢＣ") > 0);
  TEST_CHECK(STRCMP(buf, TT("ＡＢＣ")) == 0);
  TEST_MSG(sizeof(CHAR_TYPE) == 1 ? "want %s, got %s" : "want %ls, got %ls", TT("ＡＢＣ"), buf);
}

static void test_string_ls(void) {
  CHAR_TYPE buf[bufsize];
  TEST_CHECK(ov_snprintf(buf, bufsize, NULL, TT("%ls"), L"ＡＢＣ") > 0);
  TEST_CHECK(STRCMP(buf, TT("ＡＢＣ")) == 0);
  TEST_MSG(sizeof(CHAR_TYPE) == 1 ? "want %s, got %s" : "want %ls, got %ls", TT("ＡＢＣ"), buf);
}

TEST_LIST = {
    {"test1", test1},
    {"test_printf_verify_format", test_printf_verify_format},
    {"test_printf_order_extension", test_printf_order_extension},
    {"test_string_ascii", test_string_ascii},
    {"test_string_s", test_string_s},
    {"test_string_ls", test_string_ls},
    {NULL, NULL},
};
