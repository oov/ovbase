#include "wchar.c"
#include <ovbase.h>
#define STRCMP wcscmp
#define TT(str) L##str
#define STRFMT L"ls"
#include "test.inc.c"
