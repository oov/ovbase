#ifdef USE_STR
#  include "char.c"
#  define STRCMP strcmp
#  define TT(str) str
#  define STRFMT "s"
#  include "test.inc.c"
#endif // USE_STR
