#include <ovbase.h>
#ifndef OV_NOSTR
#  ifdef USE_STR
#    include "char.c"
#    define STRCMP strcmp
#    define TT(str) str
#    define STRFMT "s"
#    include "test.inc.c"
#  endif
#endif

#if defined(OV_NOSTR) || !defined(USE_STR)
#  include "ovtest.h"
TEST_LIST = {
    {NULL, NULL},
};
#endif
