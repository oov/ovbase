#include "ovbase.c"
#include "ovtest.h"

#include <inttypes.h>

struct test_mtxs {
  mtx_t m1;
  mtx_t m2;
  cnd_t c1;
  cnd_t c2;
  int v1;
  int v2;
};

static int test_mtx_timedwait_worker(void *userdata) {
  struct test_mtxs *m = userdata;
  mtx_lock(&m->m2);

  mtx_lock(&m->m1);
  m->v1 = 1;
  cnd_signal(&m->c1);
  mtx_unlock(&m->m1);

  struct timespec next = {0}, cur = {0};
  timespec_get(&next, TIME_UTC);
  next.tv_sec += 3;
  do {
    thrd_yield();
    timespec_get(&cur, TIME_UTC);
  } while (cur.tv_sec < next.tv_sec);
  m->v2 = 1;
  cnd_signal(&m->c2);
  mtx_unlock(&m->m2);
  return 0;
}

static void test_mtx_timedwait(void) {
  struct test_mtxs m = {0};
  thrd_t t = {0};
  mtx_init(&m.m1, mtx_plain);
  cnd_init(&m.c1);
  mtx_init(&m.m2, mtx_timed);
  cnd_init(&m.c2);

  mtx_lock(&m.m1);
  thrd_create(&t, test_mtx_timedwait_worker, &m);
  while (m.v1 == 0) {
    cnd_wait(&m.c1, &m.m1);
  }

  struct timespec before = {0};
  timespec_get(&before, TIME_UTC);

  struct timespec ts = {0};
  timespec_get(&ts, TIME_UTC);
  ts.tv_sec += 1;
  TEST_ASSERT(mtx_timedlock(&m.m2, &ts) == thrd_timedout);

  struct timespec after = {0};
  timespec_get(&after, TIME_UTC);
  TEST_ASSERT(after.tv_sec - before.tv_sec >= 1);
  TEST_MSG("before %" PRIu64, (uint64_t)before.tv_sec);
  TEST_MSG("after  %" PRIu64, (uint64_t)after.tv_sec);

  thrd_join(t, NULL);
  cnd_destroy(&m.c2);
  mtx_destroy(&m.m2);
  cnd_destroy(&m.c1);
  mtx_destroy(&m.m1);
}

static void test_cnd_timedwait(void) {
  mtx_t m = {0};
  cnd_t c = {0};
  mtx_init(&m, mtx_plain | mtx_recursive);
  cnd_init(&c);
  mtx_lock(&m);
  struct timespec ts = {0};
  timespec_get(&ts, TIME_UTC);
  ts.tv_sec += 1;
  TEST_ASSERT(cnd_timedwait(&c, &m, &ts) == thrd_timedout);
  mtx_unlock(&m);
  cnd_destroy(&c);
  mtx_destroy(&m);
}

TEST_LIST = {
    {"test_mtx_timedwait", test_mtx_timedwait},
    {"test_cnd_timedwait", test_cnd_timedwait},
    {NULL, NULL},
};
