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

TEST_LIST = {
    {"test1", test1},
    {NULL, NULL},
};
