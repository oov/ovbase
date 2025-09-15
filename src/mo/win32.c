#include <ovarray.h>
#include <ovmo.h>

#ifdef _WIN32

#  include <assert.h>

#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

static bool mo_get_preferred_ui_languages_core(NATIVE_CHAR **dest, bool const id, struct ov_error *const err) {
  assert(dest != NULL && "dest must not be NULL");
  if (!dest) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }

  NATIVE_CHAR *dest_prev = *dest;
  HMODULE h = NULL;
  bool result = false;

  h = LoadLibraryW(L"kernel32.dll");
  if (h == NULL) {
    OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }
  {
    typedef BOOL(WINAPI * GETHREADPERFERREDUILANGUAGESPROC)(
        DWORD dwFlags, PULONG pulNumLanguages, PZZWSTR pwszLanguagesBuffer, PULONG pcchLanguagesBuffer);
    GETHREADPERFERREDUILANGUAGESPROC fn =
        (GETHREADPERFERREDUILANGUAGESPROC)(void *)GetProcAddress(h, "GetThreadPreferredUILanguages");
    if (fn == NULL) {
      OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
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
      OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
      goto cleanup;
    }
    if (!OV_ARRAY_GROW(dest, len)) {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
      goto cleanup;
    }
    if (!fn(flag, &n, *dest, &len)) {
      OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
      goto cleanup;
    }
    OV_ARRAY_SET_LENGTH(*dest, len);
    result = true;
  }
cleanup:
  if (!result && dest_prev == NULL && *dest) {
    OV_ARRAY_DESTROY(dest);
  }
  if (h) {
    FreeLibrary(h);
    h = NULL;
  }
  return result;
}

bool mo_get_preferred_ui_languages(NATIVE_CHAR **dest, struct ov_error *const err) {
  assert(dest != NULL && "dest must not be NULL");
  if (!dest) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return false;
  }

  if (!mo_get_preferred_ui_languages_core(dest, false, err)) {
    OV_ERROR_ADD_TRACE(err);
    return false;
  }

  // replace '-' to '_'
  for (size_t i = 0, ln = OV_ARRAY_LENGTH(*dest); i < ln; ++i) {
    if ((*dest)[i] == L'-') {
      (*dest)[i] = L'_';
    }
  }

  return true;
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
  struct enumlang_context *const ctx = (struct enumlang_context *)lParam;
  if (*ctx->num == ctx->max) {
    return FALSE;
  }
  ctx->langs[(*ctx->num)++] = wLanguage;
  return TRUE;
}

static WORD
choose(WORD const *const preferred, size_t const num_preferred, WORD const *const resources, size_t num_resources) {
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

static HRSRC find_resource(HMODULE const module,
                           wchar_t const *const type,
                           wchar_t const *const name,
                           wchar_t const *const preferred_languages,
                           struct ov_error *const err) {
  // module can be NULL (Windows API will handle appropriately)
  assert(type != NULL && "type must not be NULL");
  assert(name != NULL && "name must not be NULL");
  assert(preferred_languages != NULL && "preferred_languages must not be NULL");
  if (!type || !name || !preferred_languages) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return NULL;
  }

  HMODULE h = NULL;
  HRSRC result = NULL;

  {
    h = LoadLibraryW(L"kernel32.dll");
    if (!h) {
      OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
      goto cleanup;
    }

    typedef LCID(WINAPI * LocaleNameToLCIDProc)(LPCWSTR lpName, DWORD dwFlags);
    LocaleNameToLCIDProc toLCID = (LocaleNameToLCIDProc)(void *)GetProcAddress(h, "LocaleNameToLCID");
    if (!toLCID) {
      OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
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
        OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
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
      OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
      goto cleanup;
    }
    if (num_resources == buf_size) {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
      goto cleanup;
    }
    WORD const found = choose(preferred, num_preferred, resources, num_resources);
    if (!found) {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_not_found);
      goto cleanup;
    }
    result = FindResourceExW(module, type, name, found);
    if (!result) {
      OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
      goto cleanup;
    }
  }

cleanup:
  if (h) {
    FreeLibrary(h);
    h = NULL;
  }
  return result;
}

struct mo *
mo_parse_from_resource_ex(void *const hmodule, wchar_t const *const preferred_languages, struct ov_error *const err) {
  // hmodule can be NULL (uses current process module)
  assert(preferred_languages != NULL && "preferred_languages must not be NULL");
  if (!preferred_languages) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    return NULL;
  }

  HMODULE hmod = (HMODULE)hmodule;
  struct mo *result = NULL;

  HRSRC r = NULL;
  HGLOBAL h = NULL;
  size_t sz = 0;
  void const *p = NULL;

  r = find_resource(hmod, MAKEINTRESOURCEW(10), L"MO", preferred_languages, err);
  if (!r) {
    OV_ERROR_ADD_TRACE(err);
    goto cleanup;
  }

  h = LoadResource(hmod, r);
  if (!h) {
    OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  sz = (size_t)(SizeofResource(hmod, r));
  if (!sz) {
    OV_ERROR_SET_HRESULT(err, HRESULT_FROM_WIN32(GetLastError()));
    goto cleanup;
  }

  if (sz <= 28) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
    goto cleanup;
  }

  p = LockResource(h);
  if (!p) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_unexpected);
    goto cleanup;
  }

  result = mo_parse(p, sz, err);
  if (!result) {
    OV_ERROR_ADD_TRACE(err);
    goto cleanup;
  }

cleanup:
  return result;
}

struct mo *mo_parse_from_resource(void *const hmodule, struct ov_error *const err) {
  // hmodule can be NULL (uses current process module)
  NATIVE_CHAR *langs = NULL;
  struct mo *result = NULL;

  if (!mo_get_preferred_ui_languages_core(&langs, false, err)) {
    OV_ERROR_ADD_TRACE(err);
    goto cleanup;
  }

  result = mo_parse_from_resource_ex(hmodule, langs, err);
  if (!result) {
    OV_ERROR_ADD_TRACE(err);
    goto cleanup;
  }

cleanup:
  if (langs) {
    OV_ARRAY_DESTROY(&langs);
  }

  return result;
}
#endif
