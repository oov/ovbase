#ifdef USE_STR
#  include "char.c"
#  define STRCMP strcmp
#  define TT(str) str
#  include "test.inc.c"
#endif // USE_STR
