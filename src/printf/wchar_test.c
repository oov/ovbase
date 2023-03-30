#ifdef USE_WSTR
#  include "wchar.c"
#  define STRCMP wcscmp
#  define TT(str) L##str
#  define STRFMT L"ls"
#  include "test.inc.c"
#endif // USE_WSTR
