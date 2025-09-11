#pragma once

#include <ovbase.h>

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

#define OV_HASHMAP_CREATE_DYNAMIC(item_size, cap, get_key_fn)                                                          \
  ov_hashmap_create_dynamic((item_size), (cap), (get_key_fn)MEM_FILEPOS_VALUES)
#define OV_HASHMAP_CREATE_STATIC(item_size, cap, key_size)                                                             \
  ov_hashmap_create_static((item_size), (cap), (key_size)MEM_FILEPOS_VALUES)
#define OV_HASHMAP_CREATE(item_size, cap, key)                                                                         \
  _Generic((key), size_t: ov_hashmap_create_static, default: ov_hashmap_create_dynamic)(item_size, cap, key)
#define OV_HASHMAP_DESTROY(hmp) ov_hashmap_destroy((hmp)MEM_FILEPOS_VALUES)
#define OV_HASHMAP_CLEAR(hmp) ov_hashmap_clear(hmp)
#define OV_HASHMAP_COUNT(hmp) ov_hashmap_count(hmp)
#define OV_HASHMAP_GET(hmp, key_item_ptr) ov_hashmap_get((hmp), (key_item_ptr))
#define OV_HASHMAP_SET(hmp, item_ptr) ov_hashmap_set((hmp), (item_ptr)MEM_FILEPOS_VALUES)
#define OV_HASHMAP_DELETE(hmp, key_item_ptr) ov_hashmap_delete((hmp), (key_item_ptr))
#define OV_HASHMAP_ITER(hmp, size_t_ptr, item_ptr_ptr) ov_hashmap_iter((hmp), (size_t_ptr), (void **)(item_ptr_ptr))
