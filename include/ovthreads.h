#pragma once

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
#  include <threads.h>
#else
#  ifdef __GNUC__
#    ifndef __has_warning
#      define __has_warning(x) 0
#    endif
#    pragma GCC diagnostic push
#    if __has_warning("-Wreserved-identifier")
#      pragma GCC diagnostic ignored "-Wreserved-identifier"
#    endif
#    if __has_warning("-Wreserved-id-macro")
#      pragma GCC diagnostic ignored "-Wreserved-id-macro"
#    endif
#    if __has_warning("-Wpadded")
#      pragma GCC diagnostic ignored "-Wpadded"
#    endif
#    if __has_warning("-Wdocumentation")
#      pragma GCC diagnostic ignored "-Wdocumentation"
#    endif
#  endif // __GNUC__
#  include <time.h>
#  if !defined(TIME_UTC) && defined(_WIN32)
#    define IMPLEMENT_BASE_TIMESPEC_WIN32
#    define TIME_UTC (1)
int timespec_get(struct timespec *ts, int base);
#  endif
#  define DISABLE_TLS
#  define DISABLE_CALL_ONCE
#  include "../3rd/tinycthread/source/tinycthread.h"
#  undef DISABLE_TLS
#  undef DISABLE_CALL_ONCE
#  ifdef __GNUC__
#    pragma GCC diagnostic pop
#  endif // __GNUC__
#endif   // __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)

struct cndvar {
  cnd_t cnd;
  mtx_t mtx;
  int var;
};

static inline void cndvar_init(struct cndvar *const cv) {
  mtx_init(&cv->mtx, mtx_plain | mtx_recursive);
  cnd_init(&cv->cnd);
  cv->var = 0;
}

static inline void cndvar_exit(struct cndvar *const cv) {
  cnd_destroy(&cv->cnd);
  mtx_destroy(&cv->mtx);
}

static inline void cndvar_lock(struct cndvar *const cv) { mtx_lock(&cv->mtx); }

static inline void cndvar_unlock(struct cndvar *const cv) { mtx_unlock(&cv->mtx); }

static inline void cndvar_signal(struct cndvar *const cv, int const var) {
  cv->var = var;
  cnd_signal(&cv->cnd);
}

static inline int cndvar_wait_while(struct cndvar *cv, int var) {
  while (cv->var == var) {
    int const r = cnd_wait(&cv->cnd, &cv->mtx);
    if (r != thrd_success) {
      return r;
    }
  }
  return thrd_success;
}

static inline int cndvar_wait_until(struct cndvar *cv, int var) {
  while (cv->var != var) {
    int const r = cnd_wait(&cv->cnd, &cv->mtx);
    if (r != thrd_success) {
      return r;
    }
  }
  return thrd_success;
}

static inline int cndvar_timedwait_while(struct cndvar *cv, int var, const struct timespec *abs_time) {
  while (cv->var == var) {
    int const r = cnd_timedwait(&cv->cnd, &cv->mtx, abs_time);
    if (r != thrd_success) {
      return r;
    }
  }
  return thrd_success;
}
