#include <stddef.h>

static void insertion_sort_range(size_t const begin,
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

void old_ov_sort(size_t const n,
                 int (*const compare)(size_t const idx0, size_t const idx1, void *const userdata),
                 void (*const swap)(size_t const idx0, size_t const idx1, void *const userdata),
                 void *const userdata);

void old_ov_sort(size_t const n,
                 int (*const compare)(size_t idx0, size_t idx1, void *userdata),
                 void (*const swap)(size_t idx0, size_t idx1, void *userdata),
                 void *const userdata) {
  if (n < 2 || !compare || !swap) {
    return;
  }

  enum {
    max_levels = 64,
    insertion_threshold = 16,
  };

  size_t beg[max_levels];
  size_t end[max_levels];
  size_t i = 0;

  beg[0] = 0;
  end[0] = n;

  while (i < n) {
    size_t L = beg[i];
    if (end[i] == 0 || L >= end[i]) {
      i--;
      continue;
    }
    size_t R = end[i] - 1;
    if (L >= R) {
      i--;
      continue;
    }
    if (R - L + 1 <= insertion_threshold) {
      insertion_sort_range(L, R + 1, compare, swap, userdata);
      i--;
      continue;
    }
    size_t const mid = L + ((R - L) >> 1);
    if (compare(mid, L, userdata) < 0) {
      swap(mid, L, userdata);
    }
    if (compare(R, mid, userdata) < 0) {
      swap(R, mid, userdata);
      if (compare(mid, L, userdata) < 0) {
        swap(mid, L, userdata);
      }
    }
    size_t piv_idx = mid;
    swap(L, piv_idx, userdata);
    piv_idx = L;
    while (L < R) {
      while (compare(R, piv_idx, userdata) >= 0 && L < R) {
        R--;
      }
      if (L < R) {
        swap(L, R, userdata);
        piv_idx = R;
        L++;
      }
      while (compare(L, piv_idx, userdata) <= 0 && L < R) {
        L++;
      }
      if (L < R) {
        swap(R, L, userdata);
        piv_idx = L;
        R--;
      }
    }
    beg[i + 1] = L + 1;
    end[i + 1] = end[i];
    end[i++] = L;
    if (end[i] - beg[i] > end[i - 1] - beg[i - 1]) {
      size_t tmp = beg[i];
      beg[i] = beg[i - 1];
      beg[i - 1] = tmp;
      tmp = end[i];
      end[i] = end[i - 1];
      end[i - 1] = tmp;
    }
  }
}

#if 0
static void more_old_ov_sort(size_t const n,
                        int (*const compare)(size_t idx0, size_t idx1, void *userdata),
                        void (*const swap)(size_t idx0, size_t idx1, void *userdata),
                        void *const userdata) {
  if (n < 2 || !compare || !swap) {
    return;
  }

  enum { max_levels = sizeof(size_t) * 8 };
  size_t beg[max_levels];
  size_t end[max_levels];

  size_t i = 0;
  beg[0] = 0;
  end[0] = n;

  while (i != SIZE_MAX) {
    size_t L = beg[i];
    if (end[i] == 0 || L >= end[i]) {
      i--;
      continue;
    }

    size_t R = end[i] - 1;
    if (L < R) {
      size_t piv_idx = L;
      while (L < R) {
        while (compare(R, piv_idx, userdata) >= 0 && L < R) {
          R--;
        }
        if (L < R) {
          swap(L, R, userdata);
          piv_idx = R;
          L++;
        }
        while (compare(L, piv_idx, userdata) <= 0 && L < R) {
          L++;
        }
        if (L < R) {
          swap(R, L, userdata);
          piv_idx = L;
          R--;
        }
      }

      beg[i + 1] = L + 1;
      end[i + 1] = end[i];
      end[i++] = L;

      size_t tmp;
      if (end[i] - beg[i] > end[i - 1] - beg[i - 1]) {
        tmp = beg[i];
        beg[i] = beg[i - 1];
        beg[i - 1] = tmp;
        tmp = end[i];
        end[i] = end[i - 1];
        end[i - 1] = tmp;
      }
    } else {
      i--;
    }
  }
}
#endif
