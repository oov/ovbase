#pragma once

#include <stddef.h>

/**
 * Sort elements using callback
 *
 * The algorithm sorts elements identified by their indices [0, n) while
 * delegating comparison and swap operations to the provided callbacks. The
 * sort is not stable. The caller can carry arbitrary state through
 * `userdata` to operate on custom containers or multiple arrays.
 *
 * @param n Number of elements to sort
 * @param compare Comparison callback that returns negative/zero/positive
 *                integer for less/equal/greater
 * @param swap Swap callback that exchanges elements at the specified indices
 * @param userdata Opaque pointer forwarded to callbacks for shared context
 */
void ov_sort(size_t const n,
             int (*const compare)(size_t const idx0, size_t const idx1, void *const userdata),
             void (*const swap)(size_t const idx0, size_t const idx1, void *const userdata),
             void *const userdata);

/**
 * Sort array elements using quicksort with custom comparison callback
 *
 * Similar to the standard C library qsort() function, but supports passing
 * arbitrary user data to the comparison callback. This allows the comparison
 * function to access additional context when performing comparisons.
 *
 * @param base Pointer to the array to be sorted
 * @param n Number of elements in the array
 * @param item_size Size in bytes of each element
 * @param compare Comparison callback that returns negative/zero/positive
 *                integer for less/equal/greater. Takes two elements and
 *                userdata pointer as arguments.
 * @param userdata Opaque pointer forwarded to the comparison callback for
 *                 shared context
 */
void ov_qsort(void *const base,
              size_t const n,
              size_t const item_size,
              int (*const compare)(void const *const a, void const *const b, void *const userdata),
              void *const userdata);
