#include <string.h>

#define CHAR_TYPE char
#define PREFIX ov_
#define POSTFIX _char
#define STR_LITERAL(x) x
#define STR_CMP strcmp
#define PRINTF_STR_SPEC "%s"
#include "test.inc.c"

TEST_LIST = {
    {"ov_sprintf_char", test_ov_sprintf_char},
    {"ov_vsprintf_char", test_ov_vsprintf_char},
    {"ov_sprintf_append_char", test_ov_sprintf_append_char},
    {"ov_sprintf_format_mismatch_char", test_ov_sprintf_format_mismatch_char},
    {NULL, NULL},
};
