#include <ovtest.h>

#include <ovthreads.h>

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
  struct test_mtxs *m = (struct test_mtxs *)userdata;
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
  mtx_init(&m, mtx_plain);
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

// ============================================================
// Condvar signal-loss stress test (Windows only)
//
// Verifies the fix for tinycthread's _cnd_timedwait_win32 race:
// the original code decremented mWaitersCount BEFORE re-acquiring
// the mutex, creating a window where cnd_signal sees zero waiters
// and drops the signal → deadlock.
//
// This test implements an inline mini-threadpool that mirrors
// ovpsd's threadpool.c pattern:
//   - N workers wait on cnd for dispatch, do work via atomic_fetch_add,
//     signal cnd2 when done
//   - Main dispatches work, waits on cnd2 until index >= num_params + N
//   - VirtualAlloc/VirtualFree in each task creates kernel pressure
//     that widens the race window
//
// Without the fix: deadlocks at ~100K-200K dispatches (~10% rate).
// With the fix: completes reliably.
// ============================================================

#if defined(_WIN32)
#  include <stdatomic.h>
#  include <windows.h>

enum { STRESS_NUM_THREADS = 20 };

struct stress_tp {
  mtx_t mtx;
  cnd_t cnd;  // main → workers (dispatch)
  cnd_t cnd2; // workers → main (completion)
  atomic_size_t index;
  size_t task_id;
  size_t num_params;
  int active;
};

static int stress_worker(void *const p) {
  struct stress_tp *const tp = (struct stress_tp *)p;

  // Startup barrier: announce readiness
  mtx_lock(&tp->mtx);
  atomic_fetch_add(&tp->index, 1);
  cnd_signal(&tp->cnd2);
  mtx_unlock(&tp->mtx);

  size_t task_id = 0;
  for (;;) {
    mtx_lock(&tp->mtx);
    while (tp->task_id == task_id) {
      cnd_wait(&tp->cnd, &tp->mtx);
    }
    task_id = tp->task_id;
    int const active = tp->active;
    size_t const num_params = tp->num_params;
    mtx_unlock(&tp->mtx);

    if (!active) {
      break;
    }

    for (;;) {
      size_t const idx = atomic_fetch_add(&tp->index, 1);
      if (idx < num_params) {
        void *const mem = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (mem) {
          ((volatile char *)mem)[0] = (char)idx;
          VirtualFree(mem, 0, MEM_RELEASE);
        }
        continue;
      }
      mtx_lock(&tp->mtx);
      cnd_signal(&tp->cnd2);
      mtx_unlock(&tp->mtx);
      break;
    }
  }
  return 0;
}

struct stress_watchdog {
  atomic_int progress;
  thrd_t thread;
};

static int stress_watchdog_fn(void *const p) {
  struct stress_watchdog *const w = (struct stress_watchdog *)p;
  enum { check_ms = 500, max_stall_ms = 15000 };
  int last = atomic_load(&w->progress);
  int stall = 0;
  for (;;) {
    struct timespec ts = {0, check_ms * 1000000L};
    thrd_sleep(&ts, NULL);
    int const cur = atomic_load(&w->progress);
    if (cur < 0) {
      break;
    }
    if (cur != last) {
      last = cur;
      stall = 0;
      continue;
    }
    stall += check_ms;
    if (stall >= max_stall_ms) {
      fprintf(stderr,
              "\n[DEADLOCK] test_cnd_signal_stress: "
              "stalled for %d ms at dispatch %d\n",
              max_stall_ms,
              cur);
      fflush(stderr);
      _Exit(42);
    }
  }
  return 0;
}

static void test_cnd_signal_stress(void) {
  char const *env = getenv("OVTEST_RUN_STRESS");
  if (!env || env[0] != '1' || env[1] != '\0') {
    TEST_SKIP("Set OVTEST_RUN_STRESS=1 to run.");
    return;
  }

  enum { total_dispatches = 200000 };

  struct stress_tp tp = {0};
  mtx_init(&tp.mtx, mtx_plain);
  cnd_init(&tp.cnd);
  cnd_init(&tp.cnd2);
  atomic_init(&tp.index, 0);
  tp.active = 1;

  struct stress_watchdog wd = {0};
  int wd_created = 0;
  thrd_t workers[STRESS_NUM_THREADS];
  size_t workers_created = 0;
  for (size_t i = 0; i < STRESS_NUM_THREADS; ++i) {
    if (thrd_create(&workers[i], stress_worker, &tp) != thrd_success) {
      TEST_CHECK(false);
      TEST_MSG("failed to create worker thread %zu", i);
      goto shutdown;
    }
    ++workers_created;
  }

  // Wait for all workers to enter the wait loop
  mtx_lock(&tp.mtx);
  while (atomic_load(&tp.index) < STRESS_NUM_THREADS) {
    cnd_wait(&tp.cnd2, &tp.mtx);
  }
  mtx_unlock(&tp.mtx);

  // Start watchdog
  atomic_init(&wd.progress, 0);
  if (thrd_create(&wd.thread, stress_watchdog_fn, &wd) != thrd_success) {
    TEST_CHECK(false);
    TEST_MSG("failed to create watchdog thread");
    goto shutdown;
  }
  wd_created = 1;

  // Stress loop: varying num_params 1..23 to stress different contention patterns
  for (int d = 0; d < total_dispatches; ++d) {
    size_t const np = (size_t)((d % 23) + 1);

    mtx_lock(&tp.mtx);
    tp.num_params = np;
    atomic_store(&tp.index, 0);
    ++tp.task_id;
    cnd_broadcast(&tp.cnd);
    while (atomic_load(&tp.index) < np + STRESS_NUM_THREADS) {
      cnd_wait(&tp.cnd2, &tp.mtx);
    }
    mtx_unlock(&tp.mtx);

    atomic_store(&wd.progress, d + 1);
  }

  // Stop watchdog
  atomic_store(&wd.progress, -1);

shutdown:
  if (wd_created) {
    atomic_store(&wd.progress, -1);
    thrd_join(wd.thread, NULL);
  }
  // Signal workers to exit
  mtx_lock(&tp.mtx);
  tp.active = 0;
  ++tp.task_id;
  cnd_broadcast(&tp.cnd);
  mtx_unlock(&tp.mtx);
  for (size_t i = 0; i < workers_created; ++i) {
    thrd_join(workers[i], NULL);
  }

  cnd_destroy(&tp.cnd2);
  cnd_destroy(&tp.cnd);
  mtx_destroy(&tp.mtx);
}

#else
static void test_cnd_signal_stress(void) { TEST_SKIP("Windows-only test (tinycthread condvar signal-loss race)"); }
#endif

TEST_LIST = {
    {"test_mtx_timedwait", test_mtx_timedwait},
    {"test_cnd_timedwait", test_cnd_timedwait},
    {"test_cnd_signal_stress", test_cnd_signal_stress},
    {NULL, NULL},
};
