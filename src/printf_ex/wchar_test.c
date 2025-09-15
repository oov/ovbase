#include <wchar.h>

#define CHAR_TYPE wchar_t
#define PREFIX ov_
#define POSTFIX _wchar
#define STR_LITERAL(x) L##x
#define STR_CMP wcscmp
#define PRINTF_STR_SPEC "%ls"
#include "test.inc.c"

TEST_LIST = {
    {"ov_sprintf_wchar", test_ov_sprintf_wchar},
    {"ov_vsprintf_wchar", test_ov_vsprintf_wchar},
    {"ov_sprintf_append_wchar", test_ov_sprintf_append_wchar},
    {NULL, NULL},
};
