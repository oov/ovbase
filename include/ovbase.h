#pragma once

#include <ovbase_config.h>

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __has_c_attribute
#  define __has_c_attribute(x) 0
#endif
#ifndef __has_attribute
#  define __has_attribute(x) 0
#endif
#ifndef __has_warning
#  define __has_warning(x) 0
#endif

#if __has_attribute(warn_unused_result)
#  define NODISCARD __attribute__((warn_unused_result))
#elif __has_c_attribute(nodiscard)
#  define NODISCARD [[nodiscard]]
#else
#  define NODISCARD
#endif

#if __has_attribute(noreturn)
#  define NORETURN __attribute__((noreturn))
#elif __has_c_attribute(noreturn)
#  define NORETURN [[noreturn]]
#else
#  define NORETURN
#endif

#ifdef __FILE_NAME__
#  define SOURCE_CODE_FILE_NAME __FILE_NAME__
#else
char const *ov_find_file_name(char const *s);
#  define SOURCE_CODE_FILE_NAME (ov_find_file_name(__FILE__))
#endif

struct ov_filepos {
  char const *file;
  char const *func;
  size_t line;
};

#define ERR_FILEPOS_PARAMS , struct ov_filepos const *const filepos
#define ERR_FILEPOS_VALUES                                                                                             \
  , (&(const struct ov_filepos){.file = SOURCE_CODE_FILE_NAME, .func = __func__, .line = __LINE__})
#define ERR_FILEPOS_VALUES_PASSTHRU , filepos

#ifdef ALLOCATE_LOGGER
#  define MEM_FILEPOS_PARAMS ERR_FILEPOS_PARAMS
#  define MEM_FILEPOS_VALUES ERR_FILEPOS_VALUES
#  define MEM_FILEPOS_VALUES_PASSTHRU ERR_FILEPOS_VALUES_PASSTHRU
#else
#  define MEM_FILEPOS_PARAMS
#  define MEM_FILEPOS_VALUES
#  define MEM_FILEPOS_VALUES_PASSTHRU
#endif

#ifdef _WIN32
#  define NATIVE_CHAR wchar_t
#  define NSTR(str) L##str
#  define NEWLINE NSTR("\r\n")
#else
#  define NATIVE_CHAR char
#  define NSTR(str) str
#  define NEWLINE NSTR("\n")
#endif

static inline void *ov_deconster_(void const *const ptr) {
#ifdef __GNUC__
#  pragma GCC diagnostic push
#  if __has_warning("-Wcast-qual")
#    pragma GCC diagnostic ignored "-Wcast-qual"
#  endif
  return (void *)ptr;
#  pragma GCC diagnostic pop
#else
  return (void *)ptr;
#endif // __GNUC__
}

struct error {
  int type;
  int code;
  NATIVE_CHAR *msg;
  struct ov_filepos filepos;

  struct error *next;
};
typedef struct error *error;

// error

enum err_type {
  err_type_generic = 0,
};

enum err_generic {
  err_pass_through = 0,
  err_fail = 1,
  err_unexpected = 2,
  err_invalid_arugment = 3,
  err_invalid_argument = 3,
  err_null_pointer = 4,
  err_out_of_memory = 5,
  err_not_sufficient_buffer = 6,
  err_not_found = 7,
  err_abort = 8,
  err_not_implemented_yet = 9,
};

typedef error (*error_message_mapper)(int const type, int const code, NATIVE_CHAR **const message);
NODISCARD error error_generic_message_mapper(int const type, int const code, NATIVE_CHAR **const dest);
NODISCARD error error_errno_message_mapper(int const type, int const code, NATIVE_CHAR **const dest);
#ifdef _WIN32
NODISCARD error error_win32_message_mapper(int const type, int const code, uint16_t langid, NATIVE_CHAR **const dest);
#endif
typedef void (*error_message_reporter)(error const err,
                                       NATIVE_CHAR const *const msg,
                                       struct ov_filepos const *const filepos);
void error_default_reporter(error const e, NATIVE_CHAR const *const message, struct ov_filepos const *const filepos);
void error_set_message_mapper(error_message_mapper fn);
void error_set_reporter(error_message_reporter fn);

NODISCARD error error_add_(error const parent,
                           int const type,
                           int const code,
                           NATIVE_CHAR const *const msg ERR_FILEPOS_PARAMS);
NODISCARD error error_add_i18n_(error const parent,
                                int const type,
                                int const code,
                                char const *const msg ERR_FILEPOS_PARAMS);
NODISCARD error error_add_i18nf_(error const parent,
                                 int const type,
                                 int const code ERR_FILEPOS_PARAMS,
                                 NATIVE_CHAR const *const reference,
                                 char const *const format,
                                 ...);
bool error_free_(error *const e MEM_FILEPOS_PARAMS);
NODISCARD static inline bool error_is_(error const err, int const type, int const code) {
  return err != NULL && err->type == type && err->code == code;
}
NODISCARD error error_to_string_short(error const e, NATIVE_CHAR **const dest);
NODISCARD error error_to_string(error const e, NATIVE_CHAR **const dest);
bool error_report_(error const e, NATIVE_CHAR const *const message ERR_FILEPOS_PARAMS);
bool error_report_free_(error e, NATIVE_CHAR const *const message ERR_FILEPOS_PARAMS);
bool error_report_free_i18n_(error e, char const *const msg ERR_FILEPOS_PARAMS);
bool error_report_free_i18nf_(error e ERR_FILEPOS_PARAMS,
                              NATIVE_CHAR const *const reference,
                              char const *const format,
                              ...);

#define err(type, code) (error_add_(NULL, (type), (code), NULL ERR_FILEPOS_VALUES))
#define errg(code) (err(err_type_generic, (code)))
#define efree(err_ptr) (error_free_((err_ptr)MEM_FILEPOS_VALUES))
#define emsg(type, code, struct_native_str_ptr)                                                                        \
  (error_add_(NULL, (type), (code), (struct_native_str_ptr)ERR_FILEPOS_VALUES))
#define emsg_i18n(type, code, char_ptr) (error_add_i18n_(NULL, (type), (code), (char_ptr)ERR_FILEPOS_VALUES))
#define emsg_i18nf(type, code, reference, format, ...)                                                                 \
  (error_add_i18nf_(NULL, (type), (code)ERR_FILEPOS_VALUES, (reference), (format), __VA_ARGS__))
#define ethru(parent) (error_add_((parent), err_type_generic, err_pass_through, NULL ERR_FILEPOS_VALUES))
NODISCARD static inline error eok(void) { return NULL; }
NODISCARD static inline bool esucceeded(error const err) { return err == NULL; }
NODISCARD static inline bool efailed(error const err) { return err != NULL; }
NODISCARD static inline bool eis(error const err, int const type, int const code) { return error_is_(err, type, code); }
NODISCARD static inline bool eisg(error const err, int const code) { return eis(err, err_type_generic, code); }
#define ereportmsg(err, struct_native_str_ptr) (error_report_free_((err), (struct_native_str_ptr)ERR_FILEPOS_VALUES))
#define ereportmsg_i18n(err, char_ptr) (error_report_free_i18n_((err), (char_ptr)ERR_FILEPOS_VALUES))
#define ereportmsg_i18nf(err, reference, format, ...)                                                                  \
  (error_report_free_i18nf_((err)ERR_FILEPOS_VALUES, (reference), (format), __VA_ARGS__))
#define ereport(err) (error_report_free_((err), NSTR("Error occurred.") ERR_FILEPOS_VALUES))

// Do not use eignore for normal use cases.
// ereport is appropriate for that.
static inline bool eignore(error err) {
  if (efailed(err)) {
    efree(&err);
    return false;
  }
  return true;
}

enum {
  err_type_errno = 1,
};
#define errerrno(n) (error_add_(NULL, err_type_errno, n, NULL ERR_FILEPOS_VALUES))
static inline bool eis_errno(error err, int n) { return error_is_(err, err_type_errno, n); }

#ifdef _WIN32
#  ifndef _HRESULT_DEFINED
#    ifdef __GNUC__
#      pragma GCC diagnostic push
#      if __has_warning("-Wreserved-macro-identifier")
#        pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#      endif
#      define _HRESULT_DEFINED
#      pragma GCC diagnostic pop
#    else
#      define _HRESULT_DEFINED
#    endif // __GNUC__
typedef long HRESULT;
#  endif // _HRESULT_DEFINED
enum {
  err_type_hresult = 2,
};
#  define errhr(hr) (error_add_(NULL, err_type_hresult, (int)(hr), NULL ERR_FILEPOS_VALUES))
static inline bool eis_hr(error err, HRESULT hr) { return error_is_(err, err_type_hresult, (int)hr); }
#endif // _WIN32

// mem

NODISCARD error mem_(void *const pp, size_t const n, size_t const item_size MEM_FILEPOS_PARAMS);
NODISCARD error mem_free_(void *const pp MEM_FILEPOS_PARAMS);
#define mem(pp, n, item_size) (mem_((pp), (n), (item_size)MEM_FILEPOS_VALUES))
#define mem_free(pp) (mem_free_((pp)MEM_FILEPOS_VALUES))

NODISCARD error mem_aligned_alloc_(void *const pp,
                                   size_t const n,
                                   size_t const item_size,
                                   size_t const align MEM_FILEPOS_PARAMS);
NODISCARD error mem_aligned_free_(void *const pp MEM_FILEPOS_PARAMS);
#define mem_aligned_alloc(pp, n, item_size, align)                                                                     \
  (mem_aligned_alloc_((pp), (n), (item_size), (align)MEM_FILEPOS_VALUES))
#define mem_aligned_free(pp) (mem_aligned_free_((pp)MEM_FILEPOS_VALUES))

#ifdef LEAK_DETECTOR
long mem_get_allocated_count(void);
#endif

// hash map

typedef void (*hm_get_key)(void const *const item, void const **const key, size_t *const key_bytes);

struct hmap {
  void *ptr;
  union {
    hm_get_key get_key;
    size_t size;
  };
};

NODISCARD error hmap_new_dynamic(struct hmap *const hm,
                                 size_t const item_size,
                                 size_t const cap,
                                 hm_get_key const get_key MEM_FILEPOS_PARAMS);
NODISCARD error hmap_new_static(struct hmap *const hm,
                                size_t const item_size,
                                size_t const cap,
                                size_t const key_bytes MEM_FILEPOS_PARAMS);
NODISCARD error hmap_free(struct hmap *const hm MEM_FILEPOS_PARAMS);
NODISCARD error hmap_clear(struct hmap *const hm);
NODISCARD error hmap_count(struct hmap const *const hm, size_t *const dest);
NODISCARD error hmap_get(struct hmap *const hm, void const *const key_item, void **const item);
NODISCARD error hmap_set(struct hmap *const hm, void const *const item, void **const old_item MEM_FILEPOS_PARAMS);
NODISCARD error hmap_delete(struct hmap *const hm,
                            void const *const key_item,
                            void **const old_item MEM_FILEPOS_PARAMS);
NODISCARD error hmap_scan(struct hmap *const hm,
                          bool (*iter)(void const *const item, void *const udata),
                          void *const udata);
bool hmap_iter(struct hmap *const hm, size_t *const i, void **const item);
#define hmnewd(struct_hmap_ptr, item_size, cap, get_key_fn)                                                            \
  hmap_new_dynamic((struct_hmap_ptr), (item_size), (cap), (get_key_fn)MEM_FILEPOS_VALUES)
#define hmnews(struct_hmap_ptr, item_size, cap, key_size)                                                              \
  hmap_new_static((struct_hmap_ptr), (item_size), (cap), (key_size)MEM_FILEPOS_VALUES)
#define hmfree(struct_hmap_ptr) hmap_free((struct_hmap_ptr)MEM_FILEPOS_VALUES)
#define hmclear(struct_hmap_ptr) hmap_clear((struct_hmap_ptr))
#define hmcount(struct_hmap_ptr, size_t_ptr) hmap_count((struct_hmap_ptr), (size_t_ptr))
#define hmget(struct_hmap_ptr, key_item_ptr, item_ptr_ptr)                                                             \
  hmap_get((struct_hmap_ptr), (key_item_ptr), (void **)(item_ptr_ptr))
#define hmset(struct_hmap_ptr, item_ptr, old_item_ptr_ptr)                                                             \
  hmap_set((struct_hmap_ptr), (item_ptr), (void **)(old_item_ptr_ptr)MEM_FILEPOS_VALUES)
#define hmdelete(struct_hmap_ptr, key_item_ptr, old_item_ptr_ptr)                                                      \
  hmap_delete((struct_hmap_ptr), (key_item_ptr), (void **)(old_item_ptr_ptr)MEM_FILEPOS_VALUES)
#define hmscan(struct_hmap_ptr, iter, udata_ptr) hmap_scan((struct_hmap_ptr), (iter), (udata_ptr))
#define hmiter(struct_hmap_ptr, size_t_ptr, item_ptr_ptr)                                                              \
  hmap_iter((struct_hmap_ptr), (size_t_ptr), (void **)(item_ptr_ptr))

uint64_t get_global_hint(void);

// https://xoshiro.di.unimi.it/splitmix64.c
static inline uint64_t ov_splitmix64(uint64_t x) {
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
  x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
  return x ^ (x >> 31);
}

static inline uint64_t ov_splitmix64_next(uint64_t const x) { return x + 0x9e3779b97f4a7c15; }

// https://github.com/skeeto/hash-prospector
static inline uint32_t ov_splitmix32(uint32_t x) {
  x = (x ^ (x >> 16)) * 0x7feb352d;
  x = (x ^ (x >> 15)) * 0x846ca68b;
  return x ^ (x >> 16);
}

static inline uint32_t ov_splitmix32_next(uint32_t const x) { return x + 0x9e3779b9; }

void ov_init(void);
void ov_exit(void);
