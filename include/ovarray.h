#pragma once

#include <ovbase.h>

bool ov_array_grow(void **const a, size_t const itemsize, size_t const newcap MEM_FILEPOS_PARAMS);
void ov_array_destroy(void **const a MEM_FILEPOS_PARAMS);
size_t ov_array_length(void const *const a);
void ov_array_set_length(void *const a, size_t const newlen);
size_t ov_array_capacity(void const *const a);

bool ov_array_prepare_for_push(void **const a, size_t const itemsize MEM_FILEPOS_PARAMS);
size_t ov_array_length_decrement(void *const a);

#define OV_ARRAY_GROW(aptrptr, newcap)                                                                                 \
  (ov_array_grow((void **)(aptrptr), sizeof(**aptrptr), (size_t)(newcap)MEM_FILEPOS_VALUES) ? eok()                    \
                                                                                            : errg(err_out_of_memory))
#define OV_ARRAY_DESTROY(aptrptr) ov_array_destroy((void **)(aptrptr)MEM_FILEPOS_VALUES)
#define OV_ARRAY_LENGTH(aptr) ov_array_length((void const *)aptr)
#define OV_ARRAY_SET_LENGTH(aptr, newlen) ov_array_set_length((void *)aptr, newlen)
#define OV_ARRAY_CAPACITY(aptr) ov_array_capacity((void const *)aptr)

#define OV_ARRAY_PUSH(aptrptr, item)                                                                                   \
  (ov_array_prepare_for_push((void **)(aptrptr), sizeof(**aptrptr) MEM_FILEPOS_VALUES)                                 \
       ? ((*aptrptr)[OV_ARRAY_LENGTH(*aptrptr) - 1] = (item), eok())                                                   \
       : errg(err_out_of_memory))
#define OV_ARRAY_POP(aptr) (aptr[ov_array_length_decrement(aptr)])

#define OV_ARRAY_TEMPSTR(str, n)                                                                                       \
  ((char *)((struct __attribute__((__packed__)) {                                                                      \
              struct {                                                                                                 \
                size_t len;                                                                                            \
                size_t cap;                                                                                            \
              } header;                                                                                                \
              char buf[n];                                                                                             \
            }){{.len = strlen(str)}, str}                                                                              \
                .buf))

typedef size_t ov_bitarray;

// not growable

NODISCARD error ov_bitarray_alloc(ov_bitarray **const a, size_t const len MEM_FILEPOS_PARAMS);

#define OV_BITARRAY_ALLOC(baptrptr, len) ov_bitarray_alloc((baptrptr), (size_t)(len)MEM_FILEPOS_VALUES)
#define OV_BITARRAY_FREE(baptrptr) eignore(mem_free((baptrptr)))

// growable

bool ov_bitarray_grow(ov_bitarray **const a, size_t const newcap MEM_FILEPOS_PARAMS);

#define OV_BITARRAY_GROW(baptrptr, newcap)                                                                             \
  (ov_bitarray_grow((baptrptr), (size_t)(newcap)MEM_FILEPOS_VALUES) ? eok() : errg(err_out_of_memory))
#define OV_BITARRAY_DESTROY(baptrptr) OV_ARRAY_DESTROY((baptrptr))
#define OV_BITARRAY_LENGTH(baptr) OV_ARRAY_LENGTH((baptr))
#define OV_BITARRAY_SET_LENGTH(baptr, newlen) OV_ARRAY_SET_LENGTH((baptr), (size_t)(newlen))
#define OV_BITARRAY_CAPACITY(baptr) (OV_ARRAY_CAPACITY((baptr)) * sizeof(ov_bitarray) * 8)

static inline void ov_bitarray_set(ov_bitarray *const a, size_t const index) {
  size_t const d = index / (sizeof(ov_bitarray) * 8);
  size_t const r = index % (sizeof(ov_bitarray) * 8);
  a[d] |= ((size_t)(1) << r);
}
static inline void ov_bitarray_clear(ov_bitarray *const a, size_t const index) {
  size_t const d = index / (sizeof(ov_bitarray) * 8);
  size_t const r = index % (sizeof(ov_bitarray) * 8);
  a[d] &= ~((size_t)(1) << r);
}
static inline bool ov_bitarray_get(ov_bitarray const *const a, size_t const index) {
  size_t const d = index / (sizeof(ov_bitarray) * 8);
  size_t const r = index % (sizeof(ov_bitarray) * 8);
  return (a[d] & ((size_t)(1) << r)) != 0;
}
static inline size_t ov_bitarray_length_to_bytes(size_t const bits) {
  return (bits + (sizeof(ov_bitarray) * 8) - 1) / (sizeof(ov_bitarray) * 8);
}

#define OV_BITARRAY_SET(baptr, index) ov_bitarray_set(baptr, (size_t)(index))
#define OV_BITARRAY_CLEAR(baptr, index) ov_bitarray_clear(baptr, (size_t)(index))
#define OV_BITARRAY_GET(baptr, index) ov_bitarray_get(baptr, (size_t)(index))
#define OV_BITARRAY_LENGTH_TO_BYTES(len) ov_bitarray_length_to_bytes((size_t)(len))
