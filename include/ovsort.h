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
             int (*compare)(size_t const idx0, size_t const idx1, void *const userdata),
             void (*swap)(size_t const idx0, size_t const idx1, void *const userdata),
             void *const userdata);
