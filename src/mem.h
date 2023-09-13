#pragma once
#include <ovbase.h>

#if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
void mem_log_allocated(void const *const p MEM_FILEPOS_PARAMS);
void mem_log_free(void const *const p);
#endif

bool mem_core_(void *const pp, size_t const sz MEM_FILEPOS_PARAMS);
void *ov_hm_malloc(size_t const s, void *const udata);
void *ov_hm_realloc(void *const p, size_t const s, void *const udata);
void ov_hm_free(void *const p, void *const udata);
