#include "ovbase.h"

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  if __has_warning("-Wreserved-macro-identifier")
#    pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#  endif
#  if __has_warning("-Wpadded")
#    pragma GCC diagnostic ignored "-Wpadded"
#  endif
#  if __has_warning("-Wcast-align")
#    pragma GCC diagnostic ignored "-Wcast-align"
#  endif
#  if __has_warning("-Wsign-conversion")
#    pragma GCC diagnostic ignored "-Wsign-conversion"
#  endif
#  if __has_warning("-Wextra-semi-stmt")
#    pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#  endif
#  if __has_warning("-Wimplicit-fallthrough")
#    pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#  endif
#  include "../3rd/hashmap.c/hashmap.c"
#  pragma GCC diagnostic pop
#else
#  include "../3rd/hashmap.c/hashmap.c"
#endif // __GNUC__
