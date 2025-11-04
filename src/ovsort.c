#include <ovsort.h>

// Algorithm adapted from Darel Rex Finley's public-domain "Quicksort" implementation:
// https://alienryderflex.com/quicksort/
//
// We extend the original algorithm with a median-of-three pivot selection and an
// insertion-sort fallback for small partitions to improve behaviour on nearly
// sorted or reverse-sorted datasets.

static inline void
insertion_sort_range(size_t const begin,
                     size_t const end,
                     int (*const compare)(size_t const idx0, size_t const idx1, void *const userdata),
                     void (*const swap)(size_t const idx0, size_t const idx1, void *const userdata),
                     void *const userdata) {
  for (size_t i = begin + 1; i < end; ++i) {
    size_t j = i;
    while (j > begin && compare(j, j - 1, userdata) < 0) {
      swap(j, j - 1, userdata);
      --j;
    }
  }
}

void ov_sort(size_t const n,
             int (*const compare)(size_t const idx0, size_t const idx1, void *const userdata),
             void (*const swap)(size_t const idx0, size_t const idx1, void *const userdata),
             void *const userdata) {
  if (n < 2 || !compare || !swap) {
    return;
  }

  enum {
    max_levels = sizeof(size_t) * 8,
    insertion_threshold = 16,
  };

  size_t beg[max_levels];
  size_t end[max_levels];
  size_t level = 0;

  beg[0] = 0;
  end[0] = n;

  while (level < n) {
    size_t left = beg[level];
    size_t right = end[level];

    if (left + 1 >= right) {
      --level;
      continue;
    }

    if (right - left <= insertion_threshold) {
      insertion_sort_range(left, right, compare, swap, userdata);
      --level;
      continue;
    }

    right -= 1;

    size_t const mid = left + ((right - left) >> 1);
    if (compare(mid, left, userdata) < 0) {
      swap(mid, left, userdata);
    }
    if (compare(right, left, userdata) < 0) {
      swap(right, left, userdata);
    }
    if (compare(right, mid, userdata) < 0) {
      swap(right, mid, userdata);
    }
    swap(left, mid, userdata);

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

struct qsort_context {
  unsigned char *base;
  size_t item_size;
  int (*const compare)(void const *a, void const *b, void *userdata);
  void *userdata;
};

static int qsort_compare(size_t idx0, size_t idx1, void *userdata) {
  struct qsort_context const *const ctx = (struct qsort_context const *)userdata;
  unsigned char const *const a = ctx->base + idx0 * ctx->item_size;
  unsigned char const *const b = ctx->base + idx1 * ctx->item_size;
  return ctx->compare(a, b, ctx->userdata);
}

static void qsort_swap(size_t idx0, size_t idx1, void *userdata) {
  struct qsort_context const *const ctx = (struct qsort_context const *)userdata;
  size_t const item_size = ctx->item_size;
  unsigned char *const a = ctx->base + idx0 * item_size;
  unsigned char *const b = ctx->base + idx1 * item_size;
  // Compiler optimizes this loop well, often faster than memcpy.
  unsigned char tmp;
  for (size_t i = 0; i < item_size; ++i) {
    tmp = a[i];
    a[i] = b[i];
    b[i] = tmp;
  }
}

void ov_qsort(void *const base,
              size_t const n,
              size_t const item_size,
              int (*const compare)(void const *const a, void const *const b, void *const userdata),
              void *const userdata) {
  if (!base || !compare || !item_size || n < 2) {
    return;
  }
  ov_sort(n,
          qsort_compare,
          qsort_swap,
          &(struct qsort_context){
              .base = (unsigned char *)base,
              .item_size = item_size,
              .compare = compare,
              .userdata = userdata,
          });
}
