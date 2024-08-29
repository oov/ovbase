#include <ovbase.h>
#ifdef USE_STR
#  include "char.c"
#  define STRCMP strcmp
#  define TT(str) str
#  define STRFMT "s"
#  include "test.inc.c"
#else
#  include "ovtest.h"
TEST_LIST = {
    {NULL, NULL},
};
#endif // USE_STR
