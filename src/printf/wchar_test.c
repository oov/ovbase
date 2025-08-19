#include <ovbase.h>
#ifndef OV_NOSTR
#  ifdef USE_WSTR
#    include "wchar.c"
#    define STRCMP wcscmp
#    define TT(str) L##str
#    define STRFMT L"ls"
#    include "test.inc.c"
#  endif
#endif

#if defined(OV_NOSTR) || !defined(USE_WSTR)
#  include "ovtest.h"
TEST_LIST = {
    {NULL, NULL},
};
#endif
