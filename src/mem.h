#pragma once
#include <ovbase.h>

#if defined(ALLOCATE_LOGGER) || defined(LEAK_DETECTOR)
void mem_log_allocated(void const *const p MEM_FILEPOS_PARAMS);
void mem_log_free(void const *const p MEM_FILEPOS_PARAMS);
void mem_log_realloc_validate(void const *const old_p MEM_FILEPOS_PARAMS);
void mem_log_realloc_update(void const *const new_p MEM_FILEPOS_PARAMS);
#endif

#ifdef ALLOCATE_LOGGER
void allocate_logger_init(void);
void allocate_logger_exit(void);
size_t report_leaks(void);
#endif

#ifdef LEAK_DETECTOR
void report_allocated_count(void);
#endif

bool mem_core_(void *const pp, size_t const sz MEM_FILEPOS_PARAMS);
