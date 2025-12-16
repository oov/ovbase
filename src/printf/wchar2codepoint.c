#define SRC_CHAR_TYPE wchar_t
#define STATE_TYPE struct wchar_state
#define STATE_INIT wchar_state_init
#define STATE_FEED wchar_state_feed
#define STATE_IS_COMPLETE wchar_state_is_complete
#define STATE_FEED_CAST (wchar_t)
#define VPPRINTF_SRC ov_vpprintf_wchar
#define FUNCNAME_VPPRINTF ov_vpprintf_wchar2codepoint
#define FUNCNAME_PPRINTF ov_pprintf_wchar2codepoint
#include "to_codepoint.inc.c"
