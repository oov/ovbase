#include <ovtest.h>

#include <stdio.h>

#include <ovarray.h>

#include <ovsort.h>

struct sort_item {
  int value;
  size_t original_index;
};

static void old_ov_sort(size_t n,
                        int (*compare)(size_t idx0, size_t idx1, void *userdata),
                        void (*swap)(size_t idx0, size_t idx1, void *userdata),
                        void *userdata) {
  if (n < 2 || !compare || !swap) {
    return;
  }

  enum { max_levels = 64 };
  size_t beg[max_levels];
  size_t end[max_levels];
  ptrdiff_t level = 0;

  beg[0] = 0;
  end[0] = n;

  while (level >= 0) {
    size_t left = beg[level];
    size_t right = end[level];

    if (left + 1 >= right) {
      --level;
      continue;
    }

    right -= 1;
    size_t pivot = left;

    for (;;) {
      while (left < right && compare(right, pivot, userdata) >= 0) {
        --right;
      }
      if (left < right) {
        swap(left, right, userdata);
        if (pivot == left) {
          pivot = right;
        } else if (pivot == right) {
          pivot = left;
        }
        ++left;
      }

      while (left < right && compare(left, pivot, userdata) <= 0) {
        ++left;
      }
      if (left < right) {
        swap(right, left, userdata);
        if (pivot == right) {
          pivot = left;
        } else if (pivot == left) {
          pivot = right;
        }
        --right;
      } else {
        break;
      }
    }

    if (pivot != left) {
      swap(pivot, left, userdata);
      pivot = left;
    }

    beg[level + 1] = left + 1;
    end[level + 1] = end[level];
    end[level] = left;
    ++level;

    if (end[level] - beg[level] > end[level - 1] - beg[level - 1]) {
      size_t tmp = beg[level];
      beg[level] = beg[level - 1];
      beg[level - 1] = tmp;
      tmp = end[level];
      end[level] = end[level - 1];
      end[level - 1] = tmp;
    }
  }
}

static int compare_items(size_t idx0, size_t idx1, void *userdata) {
  struct sort_item *const items = (struct sort_item *)userdata;
  struct sort_item const *const a = &items[idx0];
  struct sort_item const *const b = &items[idx1];
  if (a->value < b->value) {
    return -1;
  }
  if (a->value > b->value) {
    return 1;
  }
  if (a->original_index < b->original_index) {
    return -1;
  }
  if (a->original_index > b->original_index) {
    return 1;
  }
  return 0;
}

static void swap_items(size_t idx0, size_t idx1, void *userdata) {
  struct sort_item *const items = (struct sort_item *)userdata;
  struct sort_item const tmp = items[idx0];
  items[idx0] = items[idx1];
  items[idx1] = tmp;
}

enum dataset_kind {
  dataset_kind_random = 0,
  dataset_kind_mostly_sorted,
  dataset_kind_reverse_sorted,
};

static bool should_run_benchmarks(void) {
  char const *env = getenv("GCMZ_RUN_BENCHMARKS");
  return env && env[0] != '\0';
}

static char const *dataset_kind_name(enum dataset_kind kind) {
  switch (kind) {
  case dataset_kind_random:
    return "random";
  case dataset_kind_mostly_sorted:
    return "mostly_sorted";
  case dataset_kind_reverse_sorted:
    return "reverse_sorted";
  }

  return "unknown";
}

static void fill_dataset(struct sort_item *items, size_t count, enum dataset_kind kind, uint32_t seed) {
  for (size_t i = 0; i < count; ++i) {
    items[i].original_index = i;
  }

  if (count == 0) {
    return;
  }

  switch (kind) {
  case dataset_kind_random: {
    uint32_t state = seed;
    for (size_t i = 0; i < count; ++i) {
      state = state * UINT32_C(1664525) + UINT32_C(1013904223);
      int const value = (int)((state >> 1) & UINT32_C(0x7fffffff)) - (int)UINT32_C(0x40000000);
      items[i].value = value;
    }
    return;
  }
  case dataset_kind_mostly_sorted: {
    for (size_t i = 0; i < count; ++i) {
      items[i].value = (int)i;
    }
    if (count > 1) {
      uint32_t state = seed;
      size_t const swap_count = (count / 16) + 1;
      for (size_t i = 0; i < swap_count; ++i) {
        state = state * UINT32_C(1664525) + UINT32_C(1013904223);
        size_t const idx0 = (size_t)(state % count);
        state = state * UINT32_C(1664525) + UINT32_C(1013904223);
        size_t const idx1 = (size_t)(state % count);
        int const tmp = items[idx0].value;
        items[idx0].value = items[idx1].value;
        items[idx1].value = tmp;
      }
    }
    return;
  }
  case dataset_kind_reverse_sorted:
    for (size_t i = 0; i < count; ++i) {
      items[i].value = (int)(count - i - 1);
    }
    return;
  }

  memset(items, 0, count * sizeof(*items));
}

static void verify_dataset(char const *name, int const *values, size_t const count) {
  TEST_CASE(name);

  struct sort_item *baseline = NULL;
  struct sort_item *optimized = NULL;

  bool const ok_baseline = OV_ARRAY_GROW(&baseline, count + 1);
  bool const ok_optimized = OV_ARRAY_GROW(&optimized, count + 1);
  TEST_ASSERT(ok_baseline && ok_optimized);

  for (size_t i = 0; i < count; ++i) {
    struct sort_item const item = {
        .value = values[i],
        .original_index = i,
    };
    baseline[i] = item;
    optimized[i] = item;
  }

  old_ov_sort(count, compare_items, swap_items, baseline);
  ov_sort(count, compare_items, swap_items, optimized);

  for (size_t i = 0; i < count; ++i) {
    bool const matches =
        baseline[i].value == optimized[i].value && baseline[i].original_index == optimized[i].original_index;
    TEST_CHECK_(matches,
                "Mismatch at index %zu: old {value=%d, original_index=%zu}, new {value=%d, original_index=%zu}",
                i,
                baseline[i].value,
                baseline[i].original_index,
                optimized[i].value,
                optimized[i].original_index);
  }

  OV_ARRAY_DESTROY(&baseline);
  OV_ARRAY_DESTROY(&optimized);
}

static void test_ov_sort_matches_standard(void) {
  static int const dataset_empty[] = {0};
  static int const dataset_sorted[] = {1, 2, 3, 4, 5, 6};
  static int const dataset_reverse[] = {6, 5, 4, 3, 2, 1};
  static int const dataset_duplicates[] = {3, 1, 2, 3, 2, 1, 3, 0};
  static int const dataset_random[] = {42, -5, 17, 0, 99, -42, 17, 8, 23};
  static int const dataset_small[] = {10};
  static int const dataset_pair[] = {5, -1};

  struct {
    char const *name;
    int const *values;
    size_t count;
  } const cases[] = {
      {"sorted", dataset_sorted, sizeof(dataset_sorted) / sizeof(dataset_sorted[0])},
      {"reverse", dataset_reverse, sizeof(dataset_reverse) / sizeof(dataset_reverse[0])},
      {"duplicates", dataset_duplicates, sizeof(dataset_duplicates) / sizeof(dataset_duplicates[0])},
      {"random", dataset_random, sizeof(dataset_random) / sizeof(dataset_random[0])},
      {"single", dataset_small, sizeof(dataset_small) / sizeof(dataset_small[0])},
      {"pair", dataset_pair, sizeof(dataset_pair) / sizeof(dataset_pair[0])},
      {"empty", dataset_empty, 0},
      {NULL, NULL, 0},
  };

  for (size_t i = 0; cases[i].name != NULL; ++i) {
    verify_dataset(cases[i].name, cases[i].values, cases[i].count);
  }
}

static void
verify_generated_dataset(char const *name, size_t count, enum dataset_kind kind, uint32_t seed, size_t variations) {
  TEST_CASE(name);

  struct sort_item *base = NULL;
  struct sort_item *baseline = NULL;
  struct sort_item *optimized = NULL;

  bool ok = OV_ARRAY_GROW(&base, count + 1);
  ok = ok && OV_ARRAY_GROW(&baseline, count + 1);
  ok = ok && OV_ARRAY_GROW(&optimized, count + 1);
  TEST_ASSERT(ok);

  for (size_t variant = 0; variant < variations; ++variant) {
    uint32_t const variant_seed = seed + (uint32_t)variant;
    fill_dataset(base, count, kind, variant_seed);
    memcpy(baseline, base, count * sizeof(*baseline));
    memcpy(optimized, base, count * sizeof(*optimized));

    old_ov_sort(count, compare_items, swap_items, baseline);
    ov_sort(count, compare_items, swap_items, optimized);

    for (size_t i = 0; i < count; ++i) {
      bool const matches =
          baseline[i].value == optimized[i].value && baseline[i].original_index == optimized[i].original_index;
      TEST_CHECK_(matches,
                  "%s: mismatch at index %zu (value old=%d new=%d original old=%zu new=%zu)",
                  dataset_kind_name(kind),
                  i,
                  baseline[i].value,
                  optimized[i].value,
                  baseline[i].original_index,
                  optimized[i].original_index);
      if (!matches) {
        break;
      }
    }
  }

  OV_ARRAY_DESTROY(&optimized);
  OV_ARRAY_DESTROY(&baseline);
  OV_ARRAY_DESTROY(&base);
}

static void test_ov_sort_matches_large_datasets(void) {
  struct {
    char const *name;
    enum dataset_kind kind;
    uint32_t seed;
  } const cases[] = {
      {"large random", dataset_kind_random, UINT32_C(0xCAFEBABE)},
      {"large mostly sorted", dataset_kind_mostly_sorted, UINT32_C(0x12348765)},
      {"large reverse sorted", dataset_kind_reverse_sorted, UINT32_C(0x0F0F0F0F)},
  };

  size_t const count = 1000;
  size_t const variations = 5;

  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    verify_generated_dataset(cases[i].name, count, cases[i].kind, cases[i].seed, variations);
  }
}

struct benchmark_case {
  char const *label;
  enum dataset_kind kind;
  size_t count;
  size_t iterations;
  size_t samples;
  uint32_t seed;
};

static void benchmark_sort_case(struct benchmark_case const *config) {
  if (!config || config->count == 0 || config->iterations == 0 || config->samples == 0) {
    return;
  }

  TEST_CASE_("benchmark: %s", config->label);

  size_t const count = config->count;
  size_t const iterations = config->iterations;
  size_t const samples = config->samples;

  struct sort_item *base = NULL;
  struct sort_item *baseline = NULL;
  struct sort_item *optimized = NULL;

  bool ok = OV_ARRAY_GROW(&base, count + 1);
  ok = ok && OV_ARRAY_GROW(&baseline, count + 1);
  ok = ok && OV_ARRAY_GROW(&optimized, count + 1);
  TEST_ASSERT(ok);

  double old_total = 0.0;
  double new_total = 0.0;

  for (size_t run = 0; run < samples; ++run) {
    uint32_t const seed = config->seed + (uint32_t)run;
    fill_dataset(base, count, config->kind, seed);

    acutest_timer_get_time_(&acutest_timer_start_);
    for (size_t i = 0; i < iterations; ++i) {
      memcpy(baseline, base, count * sizeof(*baseline));
      old_ov_sort(count, compare_items, swap_items, baseline);
    }
    acutest_timer_get_time_(&acutest_timer_end_);
    old_total += acutest_timer_diff_(acutest_timer_start_, acutest_timer_end_);

    acutest_timer_get_time_(&acutest_timer_start_);
    for (size_t i = 0; i < iterations; ++i) {
      memcpy(optimized, base, count * sizeof(*optimized));
      ov_sort(count, compare_items, swap_items, optimized);
    }
    acutest_timer_get_time_(&acutest_timer_end_);
    new_total += acutest_timer_diff_(acutest_timer_start_, acutest_timer_end_);
  }

  double const old_elapsed = old_total / (double)samples;
  double const new_elapsed = new_total / (double)samples;

  memcpy(baseline, base, count * sizeof(*baseline));
  old_ov_sort(count, compare_items, swap_items, baseline);
  memcpy(optimized, base, count * sizeof(*optimized));
  ov_sort(count, compare_items, swap_items, optimized);

  bool const arrays_match = memcmp(baseline, optimized, count * sizeof(*baseline)) == 0;
  TEST_CHECK_(
      arrays_match, "Benchmark sort mismatch detected for %s (size=%zu)", dataset_kind_name(config->kind), count);

  if (new_elapsed > 0.0) {
    double const speedup = old_elapsed / new_elapsed;
    printf("[benchmark] %s kind=%s size=%zu iterations=%zu samples=%zu old=%.6f secs new=%.6f secs speedup=%.2fx\n",
           config->label,
           dataset_kind_name(config->kind),
           count,
           iterations,
           samples,
           old_elapsed,
           new_elapsed,
           speedup);
  } else {
    printf("[benchmark] %s kind=%s size=%zu iterations=%zu samples=%zu old=%.6f secs new=%.6f secs\n",
           config->label,
           dataset_kind_name(config->kind),
           count,
           iterations,
           samples,
           old_elapsed,
           new_elapsed);
  }

  OV_ARRAY_DESTROY(&optimized);
  OV_ARRAY_DESTROY(&baseline);
  OV_ARRAY_DESTROY(&base);
}

static void test_ov_sort_benchmark(void) {
  if (!should_run_benchmarks()) {
    TEST_SKIP("Skipping benchmark tests in normal test runs");
    return;
  }

  acutest_timer_init_();
  struct benchmark_case const cases[] = {
      {"size=128 kind=random", dataset_kind_random, 128, 1000, 100, UINT32_C(0x12345678)},
      {"size=128 kind=mostly_sorted", dataset_kind_mostly_sorted, 128, 1000, 100, UINT32_C(0x23456789)},
      {"size=128 kind=reverse_sorted", dataset_kind_reverse_sorted, 128, 1000, 100, UINT32_C(0x3456789A)},
      {"size=1024 kind=random", dataset_kind_random, 1024, 1000, 100, UINT32_C(0x12345678)},
      {"size=1024 kind=mostly_sorted", dataset_kind_mostly_sorted, 1024, 1000, 100, UINT32_C(0x23456789)},
      {"size=1024 kind=reverse_sorted", dataset_kind_reverse_sorted, 1024, 1000, 100, UINT32_C(0x3456789A)},
      {"size=8192 kind=random", dataset_kind_random, 8192, 50, 10, UINT32_C(0x87654321)},
      {"size=8192 kind=reverse_sorted", dataset_kind_reverse_sorted, 8192, 50, 10, UINT32_C(0x98765432)},
      {"size=102400 kind=random", dataset_kind_random, 102400, 20, 10, UINT32_C(0x13579BDF)},
      {"size=102400 kind=mostly_sorted", dataset_kind_mostly_sorted, 102400, 20, 10, UINT32_C(0x2468ACE0)},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    benchmark_sort_case(&cases[i]);
  }
  TEST_CASE_(NULL);
}

TEST_LIST = {
    {"test_ov_sort_matches_standard", test_ov_sort_matches_standard},
    {"test_ov_sort_matches_large_datasets", test_ov_sort_matches_large_datasets},
    {"test_ov_sort_benchmark", test_ov_sort_benchmark},
    {NULL, NULL},
};
