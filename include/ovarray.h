#pragma once

#include <assert.h>
#include <ovbase.h>

/**
 * @brief Grow a dynamic array to at least the specified capacity
 *
 * @param aptrptr Pointer to array pointer (will be reallocated if needed)
 * @param newcap Minimum new capacity required
 * @return true on success, false on memory allocation failure
 *
 * @note If current capacity is already >= newcap, no reallocation occurs
 *
 * @example
 * int *numbers = NULL;
 * if (OV_ARRAY_GROW(&numbers, 100)) {
 *   // Array can now hold at least 100 integers
 *   for (int i = 0; i < 50; i++) {
 *     numbers[i] = i;
 *   }
 *   OV_ARRAY_SET_LENGTH(numbers, 50);
 *   OV_ARRAY_DESTROY(&numbers); // Don't forget to free memory
 * }
 */
#define OV_ARRAY_GROW(aptrptr, newcap)                                                                                 \
  (ov_array_grow((void **)(aptrptr), sizeof(**aptrptr), (size_t)(newcap)MEM_FILEPOS_VALUES))

/**
 * @brief Destroy a dynamic array and free its memory
 *
 * @param aptrptr Pointer to array pointer (will be set to NULL after destruction)
 */
#define OV_ARRAY_DESTROY(aptrptr) (ov_array_destroy((void **)(aptrptr)MEM_FILEPOS_VALUES))

/**
 * @brief Get the current length (number of elements) of a dynamic array
 *
 * @param aptr Array pointer (NULL is allowed, returns 0)
 * @return Number of elements currently in the array, or 0 if aptr is NULL
 */
#define OV_ARRAY_LENGTH(aptr) (ov_array_length((void const *)aptr))

/**
 * @brief Set the length (number of elements) of a dynamic array
 *
 * @param aptr Array pointer (NULL is allowed, function does nothing)
 * @param newlen New length to set
 *
 * @note This does not reallocate memory. Ensure capacity is sufficient before calling.
 * @note If aptr is NULL, this function does nothing.
 */
#define OV_ARRAY_SET_LENGTH(aptr, newlen) (ov_array_set_length((void *)aptr, newlen))

/**
 * @brief Get the current capacity (maximum elements without reallocation) of a dynamic array
 *
 * @param aptr Array pointer (NULL is allowed, returns 0)
 * @return Maximum number of elements the array can hold without reallocation, or 0 if aptr is NULL
 */
#define OV_ARRAY_CAPACITY(aptr) (ov_array_capacity((void const *)aptr))

/**
 * @brief Push an item to the end of a dynamic array
 *
 * @param aptrptr Pointer to array pointer (will be reallocated if needed)
 * @param item Item to push to the array
 * @return true on success, false on memory allocation failure
 *
 * @note Automatically grows the array if needed and increments the length
 *
 * @example
 * int *numbers = NULL;
 * if (OV_ARRAY_PUSH(&numbers, 42)) {
 *   // numbers[0] is now 42
 *   // OV_ARRAY_LENGTH(numbers) is now 1
 * }
 * if (OV_ARRAY_PUSH(&numbers, 100)) {
 *   // numbers[1] is now 100
 *   // OV_ARRAY_LENGTH(numbers) is now 2
 * }
 * OV_ARRAY_DESTROY(&numbers); // Clean up when done
 */
#define OV_ARRAY_PUSH(aptrptr, item)                                                                                   \
  (ov_array_prepare_for_push((void **)(aptrptr), sizeof(**aptrptr) MEM_FILEPOS_VALUES)                                 \
       ? ((*aptrptr)[OV_ARRAY_LENGTH(*aptrptr) - 1] = (item), true)                                                    \
       : false)

/**
 * @brief Pop (remove and return) the last item from a dynamic array
 *
 * @param aptr Array pointer (must not be NULL)
 * @return The last element that was removed from the array
 *
 * @note Decrements the array length but does not deallocate memory
 * @warning Undefined behavior if array is empty or NULL
 */
#define OV_ARRAY_POP(aptr) (aptr[ov_array_length_decrement(aptr)])

/**
 * @brief Create a temporary string that looks like a dynamic array
 *
 * @param str String literal to wrap
 * @return char* pointer that can be used with OV_ARRAY_LENGTH() etc.
 *
 * @note This creates a stack-allocated structure that mimics a dynamic array
 * @note Useful for passing string literals to functions expecting dynamic arrays
 * @warning The returned pointer is only valid in the current scope
 *
 * @example
 * char *hello = OV_ARRAY_TEMPSTR("hello");
 * printf("Length: %zu\n", OV_ARRAY_LENGTH(hello)); // Prints: Length: 5
 * printf("String: %s\n", hello); // Prints: String: hello
 * // Don't store hello for later use - it's only valid in this scope!
 */
#define OV_ARRAY_TEMPSTR(str)                                                                                          \
  ((char *)((struct __attribute__((__packed__)) {                                                                      \
              struct {                                                                                                 \
                size_t len;                                                                                            \
                size_t cap;                                                                                            \
              } header;                                                                                                \
              char buf[sizeof(str)];                                                                                   \
            }){{.len = sizeof(str) - 1}, str}                                                                          \
                .buf))

NODISCARD bool ov_array_grow(void **const a, size_t const itemsize, size_t const newcap MEM_FILEPOS_PARAMS);
void ov_array_destroy(void **const a MEM_FILEPOS_PARAMS);
NODISCARD size_t ov_array_length(void const *const a);
void ov_array_set_length(void *const a, size_t const newlen);
NODISCARD size_t ov_array_capacity(void const *const a);
NODISCARD bool ov_array_prepare_for_push(void **const a, size_t const itemsize MEM_FILEPOS_PARAMS);
size_t ov_array_length_decrement(void *const a);

typedef size_t ov_bitarray;

// not growable

/**
 * @brief Allocate a fixed-size bit array
 *
 * @param baptrptr Pointer to bit array pointer (will be allocated)
 * @param len Number of bits the array should hold
 * @return true on success, false on memory allocation failure
 *
 * @note Use OV_BITARRAY_FREE() to deallocate
 *
 * @example
 * ov_bitarray *flags = NULL;
 * if (OV_BITARRAY_ALLOC(&flags, 1024)) {
 *   OV_BITARRAY_SET(flags, 10);  // Set bit 10 to 1
 *   OV_BITARRAY_SET(flags, 50);  // Set bit 50 to 1
 *   bool is_set = OV_BITARRAY_GET(flags, 10); // Returns true
 *   OV_BITARRAY_FREE(&flags);
 * }
 */
#define OV_BITARRAY_ALLOC(baptrptr, len) (ov_bitarray_alloc((baptrptr), (size_t)(len)MEM_FILEPOS_VALUES))

/**
 * @brief Free a bit array allocated with OV_BITARRAY_ALLOC()
 *
 * @param baptrptr Pointer to bit array pointer (will be set to NULL)
 */
#define OV_BITARRAY_FREE(baptrptr) (OV_FREE((baptrptr)))

NODISCARD bool ov_bitarray_alloc(ov_bitarray **const a, size_t const len MEM_FILEPOS_PARAMS);

// growable

/**
 * @brief Grow a growable bit array to at least the specified bit capacity
 *
 * @param baptrptr Pointer to bit array pointer (will be reallocated if needed)
 * @param newcap Minimum new bit capacity required
 * @return true on success, false on memory allocation failure
 *
 * @example
 * ov_bitarray *flags = NULL;
 * if (OV_BITARRAY_GROW(&flags, 2048)) {
 *   OV_BITARRAY_SET_LENGTH(flags, 1000); // Use 1000 bits
 *   OV_BITARRAY_SET(flags, 999); // Set the last bit
 *   // Array can grow further if needed
 *   OV_BITARRAY_DESTROY(&flags);
 * }
 */
#define OV_BITARRAY_GROW(baptrptr, newcap) (ov_bitarray_grow((baptrptr), (size_t)(newcap)MEM_FILEPOS_VALUES))

/**
 * @brief Destroy a growable bit array and free its memory
 *
 * @param baptrptr Pointer to bit array pointer (will be set to NULL)
 */
#define OV_BITARRAY_DESTROY(baptrptr) (OV_ARRAY_DESTROY((baptrptr)))

/**
 * @brief Get the current length (number of bits in use) of a growable bit array
 *
 * @param baptr Bit array pointer
 * @return Number of bits currently in use
 */
#define OV_BITARRAY_LENGTH(baptr) (OV_ARRAY_LENGTH((baptr)))

/**
 * @brief Set the length (number of bits in use) of a growable bit array
 *
 * @param baptr Bit array pointer
 * @param newlen New bit length to set
 */
#define OV_BITARRAY_SET_LENGTH(baptr, newlen) (OV_ARRAY_SET_LENGTH((baptr), (size_t)(newlen)))

/**
 * @brief Get the current capacity (maximum bits without reallocation) of a growable bit array
 *
 * @param baptr Bit array pointer
 * @return Maximum number of bits the array can hold without reallocation
 */
#define OV_BITARRAY_CAPACITY(baptr) (OV_ARRAY_CAPACITY((baptr)) * 8)

/**
 * @brief Set a bit to 1 in a bit array
 *
 * @param baptr Bit array pointer
 * @param index Bit index to set (0-based)
 */
#define OV_BITARRAY_SET(baptr, index) (ov_bitarray_set(baptr, (size_t)(index)))

/**
 * @brief Clear a bit to 0 in a bit array
 *
 * @param baptr Bit array pointer
 * @param index Bit index to clear (0-based)
 */
#define OV_BITARRAY_CLEAR(baptr, index) (ov_bitarray_clear(baptr, (size_t)(index)))

/**
 * @brief Get the value of a bit in a bit array
 *
 * @param baptr Bit array pointer
 * @param index Bit index to get (0-based)
 * @return true if bit is set (1), false if clear (0)
 */
#define OV_BITARRAY_GET(baptr, index) (ov_bitarray_get(baptr, (size_t)(index)))

/**
 * @brief Calculate the number of bytes needed to store the specified number of bits
 *
 * @param len Number of bits
 * @return Number of bytes required to store len bits
 */
#define OV_BITARRAY_LENGTH_TO_BYTES(len) (ov_bitarray_length_to_bytes((size_t)(len)))

NODISCARD bool ov_bitarray_grow(ov_bitarray **const a, size_t const newcap MEM_FILEPOS_PARAMS);
static inline void ov_bitarray_set(ov_bitarray *const a, size_t const index) {
  assert(a != NULL && "a must not be NULL");
  size_t const d = index / (sizeof(ov_bitarray) * 8);
  size_t const r = index % (sizeof(ov_bitarray) * 8);
  a[d] |= ((size_t)(1) << r);
}
static inline void ov_bitarray_clear(ov_bitarray *const a, size_t const index) {
  assert(a != NULL && "a must not be NULL");
  size_t const d = index / (sizeof(ov_bitarray) * 8);
  size_t const r = index % (sizeof(ov_bitarray) * 8);
  a[d] &= ~((size_t)(1) << r);
}
NODISCARD static inline bool ov_bitarray_get(ov_bitarray const *const a, size_t const index) {
  assert(a != NULL && "a must not be NULL");
  size_t const d = index / (sizeof(ov_bitarray) * 8);
  size_t const r = index % (sizeof(ov_bitarray) * 8);
  return (a[d] & ((size_t)(1) << r)) != 0;
}
NODISCARD static inline size_t ov_bitarray_length_to_bytes(size_t const bits) {
  return ((bits + (sizeof(ov_bitarray) * 8) - 1) / (sizeof(ov_bitarray) * 8)) * sizeof(ov_bitarray);
}
