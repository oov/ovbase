#pragma once

#include <ovbase.h>

#define gettext(id) mo_gettext(mo_get_default(), (id))
#define gettext_noop(id) (id)

struct mo;

bool mo_parse(struct mo **const mpp, void const *const ptr, size_t const ptrlen, struct ov_error *const err);
void mo_free(struct mo **const mpp);
char const *mo_gettext(struct mo const *const mp, char const *const id);
char const *mo_pgettext(struct mo const *const mp, char const *const ctxt, char const *const id);
char const *
mo_ngettext(struct mo const *const mp, char const *const id, char const *const id_plural, unsigned long int const n);

void mo_set_default(struct mo *const mp);
struct mo *mo_get_default(void);

bool mo_get_preferred_ui_languages(NATIVE_CHAR **const dest, struct ov_error *const err);

#ifdef _WIN32
bool mo_parse_from_resource(struct mo **const mpp, void *const hmodule, struct ov_error *const err);
bool mo_parse_from_resource_ex(struct mo **const mpp,
                               void *const hmodule,
                               wchar_t const *const preferred_languages,
                               struct ov_error *const err);
#endif
