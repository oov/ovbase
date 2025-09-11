#pragma once

#include <ovbase.h>

/**
 * @brief Create a dynamic hashmap with custom key extraction
 *
 * Creates a hashmap where keys are extracted from items using a callback function.
 * Automatically includes debug information for memory tracking.
 *
 * @param item_size Size of each item to store
 * @param cap Initial capacity (will grow as needed)
 * @param get_key_fn Function to extract key from item
 * @return Pointer to created hashmap, or NULL on failure
 *
 * @example
 *   void get_key(void const *item, void const **key, size_t *key_bytes) {
 *     struct record const *r = item;
 *     *key = &r->id;
 *     *key_bytes = sizeof(r->id);
 *   }
 *   struct ov_hashmap *hm = OV_HASHMAP_CREATE_DYNAMIC(sizeof(struct record), 64, get_key);
 */
#define OV_HASHMAP_CREATE_DYNAMIC(item_size, cap, get_key_fn)                                                          \
  ov_hashmap_create_dynamic((item_size), (cap), (get_key_fn)MEM_FILEPOS_VALUES)

/**
 * @brief Create a static key hashmap
 *
 * Creates a hashmap where keys are the first N bytes of each item.
 * Automatically includes debug information for memory tracking.
 *
 * @param item_size Size of each item to store
 * @param cap Initial capacity (will grow as needed)
 * @param key_size Number of bytes at the beginning of each item to use as key
 * @return Pointer to created hashmap, or NULL on failure
 *
 * @example
 *   struct record { int id; char name[32]; };
 *   struct ov_hashmap *hm = OV_HASHMAP_CREATE_STATIC(sizeof(struct record), 64, sizeof(int));
 */
#define OV_HASHMAP_CREATE_STATIC(item_size, cap, key_size)                                                             \
  ov_hashmap_create_static((item_size), (cap), (key_size)MEM_FILEPOS_VALUES)

/**
 * @brief Create a hashmap (auto-detect static vs dynamic)
 *
 * Automatically chooses between static and dynamic hashmap creation based on key type.
 * If key is size_t, creates static hashmap. Otherwise creates dynamic hashmap.
 *
 * @param item_size Size of each item to store
 * @param cap Initial capacity (will grow as needed)
 * @param key Either size_t for key size (static) or function pointer (dynamic)
 * @return Pointer to created hashmap, or NULL on failure
 *
 * @example
 *   // Static key (first 4 bytes)
 *   struct ov_hashmap *hm1 = OV_HASHMAP_CREATE(sizeof(struct record), 64, 4);
 *   // Dynamic key
 *   struct ov_hashmap *hm2 = OV_HASHMAP_CREATE(sizeof(struct record), 64, get_key_fn);
 */
#define OV_HASHMAP_CREATE(item_size, cap, key)                                                                         \
  _Generic((key), size_t: ov_hashmap_create_static, default: ov_hashmap_create_dynamic)(item_size, cap, key)

/**
 * @brief Destroy hashmap and free all memory
 *
 * Destroys the hashmap and sets the pointer to NULL.
 * Automatically includes debug information for memory tracking.
 *
 * @param hmp Pointer to hashmap pointer (will be set to NULL)
 *
 * @example
 *   struct ov_hashmap *hm = OV_HASHMAP_CREATE_DYNAMIC(...);
 *   OV_HASHMAP_DESTROY(&hm);  // hm is now NULL
 */
#define OV_HASHMAP_DESTROY(hmp) ov_hashmap_destroy((hmp)MEM_FILEPOS_VALUES)

/**
 * @brief Clear all items from hashmap
 *
 * Removes all items from the hashmap but keeps the hashmap itself allocated.
 *
 * @param hmp Pointer to hashmap
 *
 * @example
 *   OV_HASHMAP_CLEAR(hm);  // All items removed, hm still valid
 */
#define OV_HASHMAP_CLEAR(hmp) ov_hashmap_clear(hmp)

/**
 * @brief Get current number of items in hashmap
 *
 * @param hmp Pointer to hashmap
 * @return Number of items currently stored
 *
 * @example
 *   size_t count = OV_HASHMAP_COUNT(hm);
 */
#define OV_HASHMAP_COUNT(hmp) ov_hashmap_count(hmp)

/**
 * @brief Get item from hashmap by key
 *
 * @param hmp Pointer to hashmap
 * @param key_item_ptr Pointer to key or item containing key
 * @return Pointer to found item, or NULL if not found
 *
 * @example
 *   int key = 123;
 *   struct record *r = (struct record*)OV_HASHMAP_GET(hm, &key);
 *   if (r) { // found
 *   }
 */
#define OV_HASHMAP_GET(hmp, key_item_ptr) ov_hashmap_get((hmp), (key_item_ptr))

/**
 * @brief Set/insert item into hashmap
 *
 * Inserts item into hashmap or updates if key already exists.
 * Automatically includes debug information for memory tracking.
 *
 * @param hmp Pointer to hashmap
 * @param item_ptr Pointer to item to insert/update
 * @return true on success, false on memory allocation failure
 *
 * @example
 *   struct record r = {.id = 123, .name = "test"};
 *   if (!OV_HASHMAP_SET(hm, &r)) {
 *     // Handle memory allocation failure
 *   }
 */
#define OV_HASHMAP_SET(hmp, item_ptr) ov_hashmap_set((hmp), (item_ptr)MEM_FILEPOS_VALUES)

/**
 * @brief Delete item from hashmap
 *
 * @param hmp Pointer to hashmap
 * @param key_item_ptr Pointer to key or item containing key
 * @return Pointer to deleted item (copy of original), or NULL if not found
 *
 * @example
 *   int key = 123;
 *   struct record *deleted = (struct record*)OV_HASHMAP_DELETE(hm, &key);
 */
#define OV_HASHMAP_DELETE(hmp, key_item_ptr) ov_hashmap_delete((hmp), (key_item_ptr))

/**
 * @brief Iterate over all items in hashmap
 *
 * Use in loop to iterate through all items. Initialize iterator to 0.
 *
 * @param hmp Pointer to hashmap
 * @param size_t_ptr Pointer to iterator variable (size_t)
 * @param item_ptr_ptr Pointer to item pointer (will receive current item)
 * @return true if item retrieved, false when iteration complete
 *
 * @example
 *   size_t iter = 0;
 *   struct record *item;
 *   while (OV_HASHMAP_ITER(hm, &iter, &item)) {
 *     printf("Item: %d %s\n", item->id, item->name);
 *   }
 */
#define OV_HASHMAP_ITER(hmp, size_t_ptr, item_ptr_ptr) ov_hashmap_iter((hmp), (size_t_ptr), (void **)(item_ptr_ptr))

struct ov_hashmap;

typedef void (*ov_hashmap_get_key_func)(void const *const item, void const **const key, size_t *const key_bytes);

NODISCARD struct ov_hashmap *ov_hashmap_create_dynamic(size_t const item_size,
                                                       size_t const cap,
                                                       ov_hashmap_get_key_func const get_key MEM_FILEPOS_PARAMS);
NODISCARD struct ov_hashmap *
ov_hashmap_create_static(size_t const item_size, size_t const cap, size_t const key_bytes MEM_FILEPOS_PARAMS);
void ov_hashmap_destroy(struct ov_hashmap **const hmp MEM_FILEPOS_PARAMS);
void ov_hashmap_clear(struct ov_hashmap *const hm);
NODISCARD size_t ov_hashmap_count(struct ov_hashmap const *const hm);
NODISCARD void const *ov_hashmap_get(struct ov_hashmap const *const hm, void const *const key_item);
NODISCARD bool ov_hashmap_set(struct ov_hashmap *const hm, void const *const item MEM_FILEPOS_PARAMS);
void const *ov_hashmap_delete(struct ov_hashmap *const hm, void const *const key_item);
NODISCARD bool ov_hashmap_iter(struct ov_hashmap *const hm, size_t *const i, void **const item);
