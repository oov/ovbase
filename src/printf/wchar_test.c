#ifdef USE_WSTR
#  include "wchar.c"
#  define STRCMP wcscmp
#  define TT(str) L##str
#  define STRFMT L"ls"
#  include "test.inc.c"
#else
#  include "ovtest.h"
TEST_LIST = {
    {NULL, NULL},
};
#endif // USE_WSTR
