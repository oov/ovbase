#pragma once

#include <ovbase.h>

#define gettext(id) mo_gettext(mo_get_default(), (id))
#define gettext_noop(id) (id)

struct mo;

/**
 * @brief Parse .mo file from memory buffer
 *
 * @param ptr Pointer to .mo file data (must not be NULL)
 * @param ptrlen Size of data buffer in bytes
 * @param err Error information. Can be NULL.
 * @return Parsed mo structure on success, NULL on failure
 */
struct mo *mo_parse(void const *const ptr, size_t const ptrlen, struct ov_error *const err);

/**
 * @brief Free mo structure and set pointer to NULL
 *
 * @param mpp Pointer to mo pointer. Can be NULL or *mpp can be NULL.
 */
void mo_free(struct mo **const mpp);

/**
 * @brief Get translated string for given message ID
 *
 * @param mp Pointer to mo structure. Can be NULL.
 * @param id Message ID to translate (must not be NULL)
 * @return Translated string or original id if not found
 */
char const *mo_gettext(struct mo const *const mp, char const *const id);

/**
 * @brief Get translated string with context
 *
 * @param mp Pointer to mo structure. Can be NULL.
 * @param ctxt Context string (must not be NULL)
 * @param id Message ID to translate (must not be NULL)
 * @return Translated string or original id if not found
 */
char const *mo_pgettext(struct mo const *const mp, char const *const ctxt, char const *const id);

/**
 * @brief Get translated plural string
 *
 * @param mp Pointer to mo structure. Can be NULL.
 * @param id Singular form message ID (must not be NULL)
 * @param id_plural Plural form message ID (must not be NULL)
 * @param n Number to determine plural form
 * @return Appropriate translated string based on n
 */
char const *
mo_ngettext(struct mo const *const mp, char const *const id, char const *const id_plural, unsigned long int const n);

/**
 * @brief Set default mo structure for gettext() macro
 *
 * @param mp Pointer to mo structure to set as default. Can be NULL.
 */
void mo_set_default(struct mo *const mp);

/**
 * @brief Get current default mo structure
 *
 * @return Current default mo structure or NULL if none set
 */
struct mo *mo_get_default(void);

/**
 * @brief Get preferred UI languages from system locale
 *
 * @param dest Pointer to destination array (must not be NULL)
 * @param err Error information. Can be NULL.
 * @return true on success, false on failure
 */
bool mo_get_preferred_ui_languages(NATIVE_CHAR **const dest, struct ov_error *const err);

#ifdef _WIN32
/**
 * @brief Parse .mo file from Windows module resource (automatic language detection)
 *
 * Uses the system's preferred UI languages to automatically select the best matching
 * language resource from the module.
 *
 * @param hmodule Windows module handle. Can be NULL to use current process module.
 * @param err Error information. Can be NULL.
 * @return Parsed mo structure on success, NULL on failure
 */
struct mo *mo_parse_from_resource(void *const hmodule, struct ov_error *const err);

/**
 * @brief Parse .mo file from Windows module resource with specific languages
 *
 * Searches for .mo resources in the specified module using the provided language
 * preference list. Languages should be specified in BCP 47 format (e.g. "en-US", "ja-JP").
 *
 * @param hmodule Windows module handle. Can be NULL to use current process module.
 * @param preferred_languages Double null-terminated wide string list of language codes (must not be NULL)
 * @param err Error information. Can be NULL.
 * @return Parsed mo structure on success, NULL on failure
 *
 * @example
 *   struct ov_error err = {0};
 *   struct mo *mp = mo_parse_from_resource_ex(NULL, L"ja-JP\0en-US\0", &err);
 *   if (mp) {
 *     // Successfully loaded Japanese or English resource
 *   }
 */
struct mo *
mo_parse_from_resource_ex(void *const hmodule, wchar_t const *const preferred_languages, struct ov_error *const err);
#endif
