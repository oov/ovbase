#include "ovbase.h"

#include "ovthreads.h"

#if __STDC_VERSION__ < 201112L || defined(__STDC_NO_THREADS__)
#  if defined(IMPLEMENT_BASE_TIMESPEC_WIN32)

#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>

int timespec_get(struct timespec *ts, int base) {
  if (!ts)
    return 0;
  if (base == TIME_UTC) {
    ts->tv_sec = time(NULL);
    ts->tv_nsec = 0;
    FILETIME ft1601 = {0};
    GetSystemTimeAsFileTime(&ft1601);
    uint64_t const t =
        (ULARGE_INTEGER){
            .LowPart = ft1601.dwLowDateTime,
            .HighPart = ft1601.dwHighDateTime,
        }
            .QuadPart -
        UINT64_C(0x019DB1DED53E8000); // 1601-01-01 to 1970-01-01
    ts->tv_sec = (time_t)(t / 10000000);
    ts->tv_nsec = (long)((t % 10000000) * 100);
    return base;
  }
  return 0;
}
#  endif

#  define DISABLE_TLS
#  define DISABLE_CALL_ONCE
#  ifdef __GNUC__
#    pragma GCC diagnostic push
#    if __has_warning("-Wreserved-identifier")
#      pragma GCC diagnostic ignored "-Wreserved-identifier"
#    endif
#    if __has_warning("-Wreserved-id-macro")
#      pragma GCC diagnostic ignored "-Wreserved-id-macro"
#    endif
#    include "../3rd/tinycthread/source/tinycthread.c"
#    pragma GCC diagnostic pop
#  else
#    include "../3rd/tinycthread/source/tinycthread.c"
#  endif // __GNUC__
#  undef DISABLE_TLS
#  undef DISABLE_CALL_ONCE

#endif // __STDC_VERSION__ < 201112L || defined(__STDC_NO_THREADS__)
