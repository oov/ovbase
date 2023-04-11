#include "ovmo.h"

#ifdef _WIN32
static NODISCARD error mo_get_preferred_ui_languages_core(struct wstr *dest, bool const id) {
  error err = eok();
  HMODULE h = LoadLibraryW(L"kernel32.dll");
  if (h == NULL) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  typedef BOOL(WINAPI * GETHREADPERFERREDUILANGUAGESPROC)(
      DWORD dwFlags, PULONG pulNumLanguages, PZZWSTR pwszLanguagesBuffer, PULONG pcchLanguagesBuffer);
  GETHREADPERFERREDUILANGUAGESPROC fn =
      (GETHREADPERFERREDUILANGUAGESPROC)(void *)GetProcAddress(h, "GetThreadPreferredUILanguages");
  if (fn == NULL) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  enum {
    mui_language_id = 0x04,
    mui_language_name = 0x08,
    mui_merge_user_fallback = 0x20,
  };
  DWORD const flag = id ? mui_language_id : mui_language_name;
  ULONG n = 0, len = 0;
  if (!fn(flag, &n, NULL, &len)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  err = sgrow(dest, len);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  if (!fn(flag, &n, dest->ptr, &len)) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  dest->len = (size_t)len;
cleanup:
  if (h) {
    FreeLibrary(h);
    h = NULL;
  }
  return err;
}

NODISCARD error mo_get_preferred_ui_languages(struct NATIVE_STR *dest) {
  if (!dest) {
    return errg(err_invalid_arugment);
  }
  error err = mo_get_preferred_ui_languages_core(dest, false);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  // replace '-' to '_'
  for (size_t i = 0; i < dest->len; ++i) {
    if (dest->ptr[i] == L'-') {
      dest->ptr[i] = L'_';
    }
  }
  return eok();
}

struct enumlang_context {
  WORD *langs;
  size_t *num;
  size_t max;
};

static BOOL CALLBACK enumlang(HMODULE hModule, LPCWSTR lpType, LPCWSTR lpName, WORD wLanguage, LONG_PTR lParam) {
  (void)hModule;
  (void)lpType;
  (void)lpName;
  struct enumlang_context *const ctx = (void *)lParam;
  if (*ctx->num == ctx->max) {
    return FALSE;
  }
  ctx->langs[(*ctx->num)++] = wLanguage;
  return TRUE;
}

static WORD choose(WORD const*const preferred, size_t const num_preferred, WORD const*const resources, size_t num_resources) {
  WORD candidate = 0;
  for (size_t i = 0; i < num_preferred; ++i) {
    WORD const prefered_primary = PRIMARYLANGID(preferred[i]);
    for (size_t j = 0; j < num_resources; ++j) {
      if (resources[j] == preferred[i]) {
        return resources[j];
      }
      if (!candidate && (prefered_primary == PRIMARYLANGID(resources[j]))) {
        candidate = resources[j];
      }
    }
  }
  return candidate;
}

static NODISCARD error find_resource(HMODULE const module,
                                     wchar_t const *const type,
                                     wchar_t const *const name,
                                     wchar_t const *const preferred_languages,
                                     HRSRC *const dest) {
  if (type == NULL || name == NULL || preferred_languages == NULL || dest == NULL) {
    return errg(err_invalid_arugment);
  }
  error err = eok();
  HMODULE h = LoadLibraryW(L"kernel32.dll");
  if (h == NULL) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  typedef LCID(WINAPI * LocaleNameToLCIDProc)(LPCWSTR lpName, DWORD dwFlags);
  LocaleNameToLCIDProc toLCID = (LocaleNameToLCIDProc)(void *)GetProcAddress(h, "LocaleNameToLCID");
  if (toLCID == NULL) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  enum {
    buf_size = 256,
  };

  WORD preferred[buf_size] = {0};
  size_t num_preferred = 0;
  for (wchar_t const *l = preferred_languages; *l != L'\0'; l += wcslen(l) + 1) {
    WORD const lang = LANGIDFROMLCID(toLCID(l, 0));
    if (lang == 0) {
      continue;
    }
    if (num_preferred == buf_size) {
      err = errg(err_fail);
      goto cleanup;
    }
    preferred[num_preferred++] = lang;
  }

  WORD resources[buf_size] = {0};
  size_t num_resources = 0;
  if (!EnumResourceLanguagesW(
          module,
          type,
          name,
          enumlang,
          (LONG_PTR) & (struct enumlang_context){.langs = resources, .num = &num_resources, .max = buf_size})) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  if (num_resources == buf_size) {
    err = errg(err_fail);
    goto cleanup;
  }
  WORD const found = choose(preferred, num_preferred, resources, num_resources);
  if (!found) {
    err = errg(err_not_found);
    goto cleanup;
  }
  HRSRC r = FindResourceExW(module, type, name, found);
  if (!r) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  *dest = r;
cleanup:
  if (h) {
    FreeLibrary(h);
    h = NULL;
  }
  return err;
}

NODISCARD error mo_parse_from_resource_ex(struct mo **const mpp,
                                          HMODULE const hmod,
                                          wchar_t const *const preferred_languages) {
  HRSRC r = NULL;
  error err = find_resource(hmod, MAKEINTRESOURCEW(10), L"MO", preferred_languages, &r);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  HGLOBAL const h = LoadResource(hmod, r);
  if (!h) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  size_t const sz = (size_t)(SizeofResource(hmod, r));
  if (!sz) {
    err = errhr(HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  if (sz <= 28) {
    err = errg(err_fail);
    goto cleanup;
  }
  void const *const p = LockResource(h);
  if (!p) {
    return errg(err_unexpected);
  }
  err = mo_parse(mpp, p, sz);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  return err;
}

NODISCARD error mo_parse_from_resource(struct mo **const mpp, HMODULE const hmod) {
  struct NATIVE_STR langs = {0};
  error err = mo_get_preferred_ui_languages_core(&langs, false);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
  err = mo_parse_from_resource_ex(mpp, hmod, langs.ptr);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  ereport(sfree(&langs));
  return err;
}
#endif
