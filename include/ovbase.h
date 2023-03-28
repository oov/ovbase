#pragma once

#include <errno.h>
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

#if __has_c_attribute(nodiscard)
#  define NODISCARD [[nodiscard]]
#elif __has_attribute(warn_unused_result)
#  define NODISCARD __attribute__((warn_unused_result))
#else
#  define NODISCARD
#endif

#if __has_c_attribute(noreturn)
#  define NORETURN [[noreturn]]
#elif __has_attribute(noreturn)
#  define NORETURN __attribute__((noreturn))
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
#  define NATIVE_STR wstr
#  define NSTR(str) L##str
#  define NEWLINE NSTR("\r\n")
#  define native_unmanaged(char_ptr) (wstr_unmanaged(char_ptr))
#  define native_unmanaged_const(char_ptr) (wstr_unmanaged_const(char_ptr))
#  ifndef USE_WSTR
#    define USE_WSTR
#  endif
#else
#  define NATIVE_CHAR char
#  define NATIVE_STR str
#  define NSTR(str) str
#  define NEWLINE NSTR("\n")
#  define native_unmanaged(char_ptr) (str_unmanaged(char_ptr))
#  define native_unmanaged_const(char_ptr) (str_unmanaged_const(char_ptr))
#  ifndef USE_STR
#    define USE_STR
#  endif
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

#ifdef USE_STR
#  include <string.h> // strlen
struct str {
  char *ptr;
  size_t len;
  size_t cap;
};
#  define str_unmanaged(char_ptr) ((struct str){.ptr = (char *)(char_ptr), .len = strlen((char_ptr))})
#  define str_unmanaged_const(char_ptr)                                                                                \
    ((struct str const){.ptr = (char *)ov_deconster_((char_ptr)), .len = strlen((char_ptr))})
#endif // USE_STR

#ifdef USE_WSTR
#  include <wchar.h> // wcslen
struct wstr {
  wchar_t *ptr;
  size_t len;
  size_t cap;
};
#  define wstr_unmanaged(wchar_ptr) ((struct wstr){.ptr = (wchar_t *)(wchar_ptr), .len = wcslen((wchar_ptr))})
#  define wstr_unmanaged_const(wchar_ptr)                                                                              \
    ((struct wstr const){.ptr = (wchar_t *)ov_deconster_((wchar_ptr)), .len = wcslen((wchar_ptr))})
#endif // USE_WSTR

struct error {
  int type;
  int code;
  struct NATIVE_STR msg;
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
  err_null_pointer = 4,
  err_out_of_memory = 5,
  err_not_sufficient_buffer = 6,
  err_not_found = 7,
  err_abort = 8,
  err_not_implemented_yet = 9,
};

typedef error (*error_message_mapper)(int const code, struct NATIVE_STR *const message);
NODISCARD error error_register_message_mapper(int const type, error_message_mapper fn);
NODISCARD error generic_error_message_mapper_en(int const code, struct NATIVE_STR *const message);
NODISCARD error generic_error_message_mapper_jp(int const code, struct NATIVE_STR *const message);
typedef void (*error_message_reporter)(error err,
                                       struct NATIVE_STR const *const msg,
                                       struct ov_filepos const *const filepos);
void error_default_reporter(error const e,
                            struct NATIVE_STR const *const message,
                            struct ov_filepos const *const filepos);
void error_register_reporter(error_message_reporter fn);

NODISCARD error error_add_(error const parent,
                           int const type,
                           int const code,
                           struct NATIVE_STR const *const msg ERR_FILEPOS_PARAMS);
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
NODISCARD error error_to_string_short(error const e, struct NATIVE_STR *const dest);
NODISCARD error error_to_string(error const e, struct NATIVE_STR *const dest);
bool error_report_(error const e, struct NATIVE_STR const *const message ERR_FILEPOS_PARAMS);
bool error_report_free_(error e, struct NATIVE_STR const *const message ERR_FILEPOS_PARAMS);
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
#define ereport(err) (error_report_free_((err), &native_unmanaged(NSTR("Error occurred.")) ERR_FILEPOS_VALUES))

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

// array

struct array {
  void *ptr; // (ptr != NULL && cap == 0) is unmanaged memory
  size_t len;
  size_t cap;
};

bool array_grow_core_(struct array *const p, size_t const elem_size, size_t const least_size MEM_FILEPOS_PARAMS);
NODISCARD error array_grow_(struct array *const p, size_t const elem_size, size_t const least_size MEM_FILEPOS_PARAMS);
void array_free_core_(struct array *const p MEM_FILEPOS_PARAMS);
NODISCARD error array_free_(struct array *const p MEM_FILEPOS_PARAMS);
static inline size_t array_len_(struct array const *const p) { return p ? p->len : 0; }
static inline size_t array_cap_(struct array const *const p) { return p ? p->cap : 0; }
#define alen(array_ptr) (array_len_((struct array const *)(array_ptr)))
#define acap(array_ptr) (array_cap_((struct array const *)(array_ptr)))
#define afree(array_ptr) (array_free_((struct array *)(array_ptr)MEM_FILEPOS_VALUES))
#define agrow(array_ptr, least_size)                                                                                   \
  (array_grow_((struct array *)(array_ptr), sizeof(*(array_ptr)->ptr), (size_t)(least_size)MEM_FILEPOS_VALUES))
#define apush(array_ptr, item)                                                                                         \
  ((array_ptr) ? !((array_ptr)->ptr && !(array_ptr)->cap)                                                              \
                     ? array_grow_core_((struct array *)(array_ptr),                                                   \
                                        sizeof(*(array_ptr)->ptr),                                                     \
                                        (array_ptr)->len + 1 MEM_FILEPOS_VALUES)                                       \
                           ? ((array_ptr)->ptr[(array_ptr)->len++] = (item), eok())                                    \
                           : errg(err_out_of_memory)                                                                   \
                     : errg(err_unexpected)                                                                            \
               : errg(err_invalid_arugment))
#define apop(array_ptr, item_ptr)                                                                                      \
  ((array_ptr) ? (array_ptr)->len ? (*(item_ptr) = (array_ptr)->ptr[--(array_ptr)->len], eok()) : errg(err_not_found)  \
               : errg(err_invalid_arugment))
#define achop(array_ptr)                                                                                               \
  ((array_ptr) ? (array_ptr)->len ? (--(array_ptr)->len, eok()) : errg(err_not_found) : errg(err_invalid_arugment))

// str

#ifndef OV_PRINTF_ATTR
#  if 0
format annotation does not support '%1$s'
#    ifdef __GNUC__
#      define OV_PRINTF_ATTR(FUNC, FORMAT, VARGS) __attribute__((format(FUNC, FORMAT, VARGS)))
#    else
#      define OV_PRINTF_ATTR(FUNC, FORMAT, VARGS)
#    endif
#  else
#    define OV_PRINTF_ATTR(FUNC, FORMAT, VARGS)
#  endif
#endif

#ifdef USE_STR
NODISCARD static inline error str_free_(struct str *const s MEM_FILEPOS_PARAMS) {
  return array_free_((struct array *)s MEM_FILEPOS_VALUES_PASSTHRU);
}
NODISCARD static inline error str_grow_(struct str *const s, size_t const cap MEM_FILEPOS_PARAMS) {
  return array_grow_((struct array *)s, sizeof(*s->ptr), cap MEM_FILEPOS_VALUES_PASSTHRU);
}
NODISCARD error str_cpy_(struct str *const s, char const *const s2 MEM_FILEPOS_PARAMS);
NODISCARD error str_cpy_m_(struct str *const s, char const *const *const s2 MEM_FILEPOS_PARAMS);
NODISCARD error str_ncpy_(struct str *const s, char const *const s2, size_t s2len MEM_FILEPOS_PARAMS);
NODISCARD error str_cat_(struct str *const s, char const *const s2 MEM_FILEPOS_PARAMS);
NODISCARD error str_cat_m_(struct str *const s, char const *const *const s2 MEM_FILEPOS_PARAMS);
NODISCARD error str_ncat_(struct str *const s, char const *const s2, size_t s2len MEM_FILEPOS_PARAMS);
NODISCARD error str_str_(struct str const *const s, char const *const s2, ptrdiff_t *pos);
NODISCARD error str_replace_all_(struct str *const s,
                                 char const *const find,
                                 char const *const replacement MEM_FILEPOS_PARAMS);

NODISCARD error str_atoi_(struct str const *const s, int64_t *const dest);
NODISCARD error str_atou_(struct str const *const s, uint64_t *const dest);
NODISCARD error str_itoa_(int64_t v, struct str *const dest MEM_FILEPOS_PARAMS);
NODISCARD error str_utoa_(uint64_t v, struct str *const dest MEM_FILEPOS_PARAMS);

NODISCARD error
str_sprintf_(struct str *const dest MEM_FILEPOS_PARAMS, char const *const reference, char const *const format, ...)
#  ifdef ALLOCATE_LOGGER
    OV_PRINTF_ATTR(printf, 4, 5)
#  else
    OV_PRINTF_ATTR(printf, 3, 4)
#  endif
        ;
NODISCARD error str_vsprintf_(struct str *const dest MEM_FILEPOS_PARAMS,
                              char const *const reference,
                              char const *const format,
                              va_list valist)
#  ifdef ALLOCATE_LOGGER
    OV_PRINTF_ATTR(printf, 4, 0)
#  else
    OV_PRINTF_ATTR(printf, 3, 0)
#  endif
        ;

NODISCARD error to_str_(wchar_t const *const src, size_t const src_len, struct str *const dest MEM_FILEPOS_PARAMS);

#endif

// wstr

#ifdef USE_WSTR
NODISCARD static inline error wstr_free_(struct wstr *const ws MEM_FILEPOS_PARAMS) {
  return array_free_((struct array *)ws MEM_FILEPOS_VALUES_PASSTHRU);
}
NODISCARD static inline error wstr_grow_(struct wstr *const ws, size_t const cap MEM_FILEPOS_PARAMS) {
  return array_grow_((struct array *)ws, sizeof(*ws->ptr), cap MEM_FILEPOS_VALUES_PASSTHRU);
}
NODISCARD error wstr_cpy_(struct wstr *const ws, wchar_t const *const ws2 MEM_FILEPOS_PARAMS);
NODISCARD error wstr_cpy_m_(struct wstr *const ws, wchar_t const *const *const ws2 MEM_FILEPOS_PARAMS);
NODISCARD error wstr_ncpy_(struct wstr *const ws, wchar_t const *const ws2, size_t ws2len MEM_FILEPOS_PARAMS);
NODISCARD error wstr_cat_(struct wstr *const ws, wchar_t const *const ws2 MEM_FILEPOS_PARAMS);
NODISCARD error wstr_cat_m_(struct wstr *const ws, wchar_t const *const *const ws2 MEM_FILEPOS_PARAMS);
NODISCARD error wstr_ncat_(struct wstr *const ws, wchar_t const *const ws2, size_t ws2len MEM_FILEPOS_PARAMS);
NODISCARD error wstr_str_(struct wstr const *const ws, wchar_t const *const ws2, ptrdiff_t *pos);
NODISCARD error wstr_replace_all_(struct wstr *const ws,
                                  wchar_t const *const find,
                                  wchar_t const *const replacement MEM_FILEPOS_PARAMS);

NODISCARD error wstr_atoi_(struct wstr const *const s, int64_t *const dest);
NODISCARD error wstr_atou_(struct wstr const *const s, uint64_t *const dest);
NODISCARD error wstr_itoa_(int64_t v, struct wstr *const dest MEM_FILEPOS_PARAMS);
NODISCARD error wstr_utoa_(uint64_t v, struct wstr *const dest MEM_FILEPOS_PARAMS);

// format annotation unavailable for wprintf
NODISCARD error wstr_sprintf_(struct wstr *const dest MEM_FILEPOS_PARAMS,
                              wchar_t const *const reference,
                              wchar_t const *const format,
                              ...)
    // #  ifdef ALLOCATE_LOGGER
    //     OV_PRINTF_ATTR(wprintf, 4, 5)
    // #  else
    //     OV_PRINTF_ATTR(wprintf, 3, 4)
    // #  endif
    ;
NODISCARD error wstr_vsprintf_(struct wstr *const dest MEM_FILEPOS_PARAMS,
                               wchar_t const *const reference,
                               wchar_t const *const format,
                               va_list valist)
    // #  ifdef ALLOCATE_LOGGER
    //     OV_PRINTF_ATTR(wprintf, 4, 0)
    // #  else
    //     OV_PRINTF_ATTR(wprintf, 3, 0)
    // #  endif
    ;

NODISCARD error to_wstr_(char const *const src, size_t const src_len, struct wstr *const dest MEM_FILEPOS_PARAMS);

#endif

#if defined(USE_STR) && defined(USE_WSTR)
static inline NODISCARD error wstr_to_str_(struct wstr const *const src, struct str *const dest MEM_FILEPOS_PARAMS) {
  return to_str_(src->ptr, src->len, dest MEM_FILEPOS_VALUES_PASSTHRU);
}
static inline NODISCARD error str_to_wstr_(struct str const *const src, struct wstr *const dest MEM_FILEPOS_PARAMS) {
  return to_wstr_(src->ptr, src->len, dest MEM_FILEPOS_VALUES_PASSTHRU);
}
#endif

#define OV_GENERIC_CASE(typ, fn)                                                                                       \
  typ:                                                                                                                 \
  fn

#if defined(USE_STR) && defined(USE_WSTR)
#  define sfree(struct_str_ptr)                                                                                        \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_free_), OV_GENERIC_CASE(struct str *, str_free_))(  \
        (struct_str_ptr)MEM_FILEPOS_VALUES)
#  define sgrow(struct_str_ptr, cap)                                                                                   \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_grow_), OV_GENERIC_CASE(struct str *, str_grow_))(  \
        (struct_str_ptr), (cap)MEM_FILEPOS_VALUES)
#  define scpy(struct_str_ptr, char_ptr)                                                                               \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_cpy_), OV_GENERIC_CASE(struct str *, str_cpy_))(    \
        (struct_str_ptr), (char_ptr)MEM_FILEPOS_VALUES)
#  define scpym(struct_str_ptr, ...)                                                                                   \
    _Generic(                                                                                                          \
        (struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_cpy_m_), OV_GENERIC_CASE(struct str *, str_cpy_m_))(     \
        (struct_str_ptr), (void *)(void const *[]){__VA_ARGS__, NULL} MEM_FILEPOS_VALUES)
#  define sncpy(struct_str_ptr, char_ptr, size_t)                                                                      \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_ncpy_), OV_GENERIC_CASE(struct str *, str_ncpy_))(  \
        (struct_str_ptr), (char_ptr), (size_t)MEM_FILEPOS_VALUES)
#  define scat(struct_str_ptr, char_ptr)                                                                               \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_cat_), OV_GENERIC_CASE(struct str *, str_cat_))(    \
        (struct_str_ptr), (char_ptr)MEM_FILEPOS_VALUES)
#  define scatm(struct_str_ptr, ...)                                                                                   \
    _Generic(                                                                                                          \
        (struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_cat_m_), OV_GENERIC_CASE(struct str *, str_cat_m_))(     \
        (struct_str_ptr), (void *)(void const *[]){__VA_ARGS__, NULL} MEM_FILEPOS_VALUES)
#  define sncat(struct_str_ptr, char_ptr, size_t)                                                                      \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_ncat_), OV_GENERIC_CASE(struct str *, str_ncat_))(  \
        (struct_str_ptr), (char_ptr), (size_t)MEM_FILEPOS_VALUES)
#  define sstr(struct_str_ptr, char_ptr, ptrdiff_t_ptr)                                                                \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_str_),                                                                \
             OV_GENERIC_CASE(struct wstr const *, wstr_str_),                                                          \
             OV_GENERIC_CASE(struct str *, str_str_),                                                                  \
             OV_GENERIC_CASE(struct str const *, str_str_))((struct_str_ptr), (char_ptr), (ptrdiff_t_ptr))
#  define sreplace_all(struct_str_ptr, char_ptr_find, char_ptr_replacement)                                            \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_replace_all_),                                                        \
             OV_GENERIC_CASE(struct str *, str_replace_all_))(                                                         \
        (struct_str_ptr), (char_ptr_find), (char_ptr_replacement)MEM_FILEPOS_VALUES)
#  define satoi(struct_str_ptr, int64_t_ptr)                                                                           \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_atoi_),                                                               \
             OV_GENERIC_CASE(struct wstr const *, wstr_atoi_),                                                         \
             OV_GENERIC_CASE(struct str *, str_atoi_),                                                                 \
             OV_GENERIC_CASE(struct str const *, str_atoi_))((struct_str_ptr), (int64_t_ptr))
#  define satou(struct_str_ptr, uint64_t_ptr)                                                                          \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_atou_),                                                               \
             OV_GENERIC_CASE(struct wstr const *, wstr_atou_),                                                         \
             OV_GENERIC_CASE(struct str *, str_atou_),                                                                 \
             OV_GENERIC_CASE(struct str const *, str_atou_))((struct_str_ptr), (uint64_t_ptr))
#  define sitoa(int64_t, struct_str_ptr)                                                                               \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_itoa_), OV_GENERIC_CASE(struct str *, str_itoa_))(  \
        (int64_t), (struct_str_ptr)MEM_FILEPOS_VALUES)
#  define sutoa(uint64_t, struct_str_ptr)                                                                              \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_utoa_), OV_GENERIC_CASE(struct str *, str_utoa_))(  \
        (uint64_t), (struct_str_ptr)MEM_FILEPOS_VALUES)
#  define ssprintf(struct_str_ptr, reference, format, ...)                                                             \
    _Generic(                                                                                                          \
        (struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_sprintf_), OV_GENERIC_CASE(struct str *, str_sprintf_))( \
        (struct_str_ptr)MEM_FILEPOS_VALUES, (reference), (format), __VA_ARGS__)
#  define svsprintf(struct_str_ptr, reference, format, va_list)                                                        \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_vsprintf_),                                                           \
             OV_GENERIC_CASE(struct str *, str_vsprintf_))(                                                            \
        (struct_str_ptr)MEM_FILEPOS_VALUES, (reference), (format), (va_list))

#  define to_str(wchar_ptr_or_struct_wstr_ptr, ...)                                                                    \
    _Generic((wchar_ptr_or_struct_wstr_ptr),                                                                           \
             OV_GENERIC_CASE(struct wstr *, wstr_to_str_),                                                             \
             OV_GENERIC_CASE(struct wstr const *, wstr_to_str_),                                                       \
             OV_GENERIC_CASE(wchar_t *, to_str_),                                                                      \
             OV_GENERIC_CASE(wchar_t const *, to_str_))((wchar_ptr_or_struct_wstr_ptr),                                \
                                                        __VA_ARGS__ MEM_FILEPOS_VALUES)
#  define to_wstr(char_ptr_or_struct_str_ptr, ...)                                                                     \
    _Generic((char_ptr_or_struct_str_ptr),                                                                             \
             OV_GENERIC_CASE(struct str *, str_to_wstr_),                                                              \
             OV_GENERIC_CASE(struct str const *, str_to_wstr_),                                                        \
             OV_GENERIC_CASE(char *, to_wstr_),                                                                        \
             OV_GENERIC_CASE(char const *, to_wstr_))((char_ptr_or_struct_str_ptr), __VA_ARGS__ MEM_FILEPOS_VALUES)

#elif defined(USE_STR)
#  define sfree(struct_str_ptr)                                                                                        \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct str *, str_free_))((struct_str_ptr)MEM_FILEPOS_VALUES)
#  define sgrow(struct_str_ptr, cap)                                                                                   \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct str *, str_grow_))((struct_str_ptr), (cap)MEM_FILEPOS_VALUES)
#  define scpy(struct_str_ptr, char_ptr)                                                                               \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct str *, str_cpy_))((struct_str_ptr), (char_ptr)MEM_FILEPOS_VALUES)
#  define scpym(struct_str_ptr, ...)                                                                                   \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct str *, str_cpy_m_))(                                             \
        (struct_str_ptr), (char const *[]){__VA_ARGS__, NULL} MEM_FILEPOS_VALUES)
#  define sncpy(struct_str_ptr, char_ptr, size_t)                                                                      \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct str *, str_ncpy_))((struct_str_ptr), (char_ptr), (size_t)MEM_FILEPOS_VALUES)
#  define scat(struct_str_ptr, char_ptr)                                                                               \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct str *, str_cat_))((struct_str_ptr), (char_ptr)MEM_FILEPOS_VALUES)
#  define scatm(struct_str_ptr, ...)                                                                                   \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct str *, str_cat_m_))(                                             \
        (struct_str_ptr), (char const *[]){__VA_ARGS__, NULL} MEM_FILEPOS_VALUES)
#  define sncat(struct_str_ptr, char_ptr, size_t)                                                                      \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct str *, str_ncat_))((struct_str_ptr), (char_ptr), (size_t)MEM_FILEPOS_VALUES)
#  define sstr(struct_str_ptr, char_ptr, ptrdiff_t_ptr)                                                                \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct str const *, str_str_),                                                            \
             OV_GENERIC_CASE(struct str *, str_str_))((struct_str_ptr), (char_ptr), (ptrdiff_t_ptr))
#  define sreplace_all(struct_str_ptr, char_ptr_find, char_ptr_replacement)                                            \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct str *, str_replace_all_))(                                       \
        (struct_str_ptr), (char_ptr_find), (char_ptr_replacement)MEM_FILEPOS_VALUES)
#  define satoi(struct_str_ptr, int64_t_ptr)                                                                           \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct str *, str_atoi_),                                                                 \
             OV_GENERIC_CASE(struct str const *, str_atoi_))((struct_str_ptr), (int64_t_ptr))
#  define satou(struct_str_ptr, uint64_t_ptr)                                                                          \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct str *, str_atou_),                                                                 \
             OV_GENERIC_CASE(struct str const *, str_atou_))((struct_str_ptr), (uint64_t_ptr))
#  define sitoa(int64_t, struct_str_ptr)                                                                               \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct str *, str_itoa_))((int64_t), (struct_str_ptr)MEM_FILEPOS_VALUES)
#  define sutoa(uint64_t, struct_str_ptr)                                                                              \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct str *, str_utoa_))((uint64_t), (struct_str_ptr)MEM_FILEPOS_VALUES)
#  define ssprintf(struct_str_ptr, format, ...)                                                                        \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct str *, str_sprintf_))((struct_str_ptr)MEM_FILEPOS_VALUES, format, __VA_ARGS__)
#  define svsprintf(struct_str_ptr, format, va_list)                                                                   \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct str *, str_vsprintf_))((struct_str_ptr)MEM_FILEPOS_VALUES, format, va_list)

#  define to_str(wchar_ptr, ...)                                                                                       \
    _Generic((wchar_ptr), OV_GENERIC_CASE(wchar_t *, to_str_), OV_GENERIC_CASE(wchar_t const *, to_str_))(             \
        (wchar_ptr), __VA_ARGS__ MEM_FILEPOS_VALUES)

#elif defined(USE_WSTR)
#  define sfree(struct_str_ptr)                                                                                        \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_free_))((struct_str_ptr)MEM_FILEPOS_VALUES)
#  define sgrow(struct_str_ptr, cap)                                                                                   \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_grow_))((struct_str_ptr), (cap)MEM_FILEPOS_VALUES)
#  define scpy(struct_str_ptr, char_ptr)                                                                               \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_cpy_))((struct_str_ptr),                            \
                                                                          (char_ptr)MEM_FILEPOS_VALUES)
#  define scpym(struct_str_ptr, ...)                                                                                   \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_cpy_m_))(                                           \
        (struct_str_ptr), (wchar_t const *[]){__VA_ARGS__, NULL} MEM_FILEPOS_VALUES)
#  define sncpy(struct_str_ptr, char_ptr, size_t)                                                                      \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_ncpy_))((struct_str_ptr), (char_ptr), (size_t)MEM_FILEPOS_VALUES)
#  define scat(struct_str_ptr, char_ptr)                                                                               \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_cat_))((struct_str_ptr),                            \
                                                                          (char_ptr)MEM_FILEPOS_VALUES)
#  define scatm(struct_str_ptr, ...)                                                                                   \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_cat_m_))(                                           \
        (struct_str_ptr), (wchar_t const *[]){__VA_ARGS__, NULL} MEM_FILEPOS_VALUES)
#  define sncat(struct_str_ptr, char_ptr, size_t)                                                                      \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_ncat_))((struct_str_ptr), (char_ptr), (size_t)MEM_FILEPOS_VALUES)
#  define sstr(struct_str_ptr, char_ptr, ptrdiff_t_ptr)                                                                \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr const *, wstr_str_),                                                          \
             OV_GENERIC_CASE(struct wstr *, wstr_str_))((struct_str_ptr), (char_ptr), (ptrdiff_t_ptr))
#  define sreplace_all(struct_str_ptr, char_ptr_find, char_ptr_replacement)                                            \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_replace_all_))(                                     \
        (struct_str_ptr), (char_ptr_find), (char_ptr_replacement)MEM_FILEPOS_VALUES)
#  define satoi(struct_str_ptr, int64_t_ptr)                                                                           \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_atoi_),                                                               \
             OV_GENERIC_CASE(struct wstr const *, wstr_atoi_))((struct_str_ptr), (int64_t_ptr))
#  define satou(struct_str_ptr, uint64_t_ptr)                                                                          \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_atou_),                                                               \
             OV_GENERIC_CASE(struct wstr const *, wstr_atou_))((struct_str_ptr), (uint64_t_ptr))
#  define sitoa(int64_t, struct_str_ptr)                                                                               \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_itoa_))((int64_t),                                  \
                                                                           (struct_str_ptr)MEM_FILEPOS_VALUES)
#  define sutoa(uint64_t, struct_str_ptr)                                                                              \
    _Generic((struct_str_ptr), OV_GENERIC_CASE(struct wstr *, wstr_utoa_))((uint64_t),                                 \
                                                                           (struct_str_ptr)MEM_FILEPOS_VALUES)
#  define ssprintf(struct_str_ptr, format, ...)                                                                        \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_sprintf_))((struct_str_ptr)MEM_FILEPOS_VALUES, format, __VA_ARGS__)
#  define svsprintf(struct_str_ptr, format, va_list)                                                                   \
    _Generic((struct_str_ptr),                                                                                         \
             OV_GENERIC_CASE(struct wstr *, wstr_vsprintf_))((struct_str_ptr)MEM_FILEPOS_VALUES, format, va_list)

#  define to_wstr(char_ptr, ...)                                                                                       \
    _Generic((char_ptr), OV_GENERIC_CASE(char *, to_wstr_), OV_GENERIC_CASE(char const *, to_wstr_))(                  \
        (char_ptr), __VA_ARGS__ MEM_FILEPOS_VALUES)

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

NODISCARD bool ov_init(error_message_mapper generic_error_message_mapper);
void ov_exit(void);
