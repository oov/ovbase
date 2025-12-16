#define SRC_CHAR_TYPE char
#define STATE_TYPE struct utf8_state
#define STATE_INIT utf8_state_init
#define STATE_FEED utf8_state_feed
#define STATE_IS_COMPLETE utf8_state_is_complete
#define STATE_FEED_CAST (uint8_t)
#define VPPRINTF_SRC ov_vpprintf_char
#define FUNCNAME_VPPRINTF ov_vpprintf_char2codepoint
#define FUNCNAME_PPRINTF ov_pprintf_char2codepoint
#include "to_codepoint.inc.c"
