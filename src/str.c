#include "ovbase.h"

#ifdef USE_STR

NODISCARD static inline error str_grow(struct str *const s, size_t const least_size MEM_FILEPOS_PARAMS) {
  return array_grow_((struct array *)s, sizeof(char), least_size MEM_FILEPOS_VALUES_PASSTHRU);
}

error str_cpy_(struct str *const s, const char *s2 MEM_FILEPOS_PARAMS) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }

  size_t const s2len = strlen(s2);
  error err = str_grow(s, s2len + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  strcpy(s->ptr, s2);
  s->len = s2len;
  return eok();
}

error str_cpy_m_(struct str *const s, char const *const *const s2 MEM_FILEPOS_PARAMS) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }
  error err = eok();
  for (size_t i = 0; s2[i] != NULL; ++i) {
    err = (i == 0 ? str_cpy_ : str_cat_)(s, s2[i] MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(err)) {
      err = ethru(err);
      return err;
    }
  }
  return err;
}

error str_ncpy_(struct str *const s, char const *const s2, size_t const s2len MEM_FILEPOS_PARAMS) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }

  error err = str_grow(s, s2len + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  strncpy(s->ptr, s2, s2len);
  s->ptr[s2len] = '\0';
  s->len = strlen(s->ptr);
  return eok();
}

error str_cat_(struct str *const s, char const *const s2 MEM_FILEPOS_PARAMS) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }

  size_t const s2len = strlen(s2);
  error err = str_grow(s, s->len + s2len + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  strcat(s->ptr, s2);
  s->len += s2len;
  return eok();
}

error str_cat_m_(struct str *const s, char const *const *const s2 MEM_FILEPOS_PARAMS) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }
  error err = eok();
  for (size_t i = 0; s2[i] != NULL; ++i) {
    err = str_cat_(s, s2[i] MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(err)) {
      err = ethru(err);
      return err;
    }
  }
  return err;
}

error str_ncat_(struct str *const s, char const *const s2, size_t const s2len MEM_FILEPOS_PARAMS) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }

  error err = str_grow(s, s->len + s2len + 1 MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    return err;
  }
  strncat(s->ptr, s2, s2len);
  s->len += strlen(s->ptr + s->len);
  return eok();
}

error str_str_(struct str const *const s, char const *const s2, ptrdiff_t *const pos) {
  if (!s || !s2) {
    return errg(err_invalid_arugment);
  }
  if (!pos) {
    return errg(err_null_pointer);
  }
  if (!s->ptr || !s->len) {
    *pos = -1;
    return eok();
  }
  char const *const found = strstr(s->ptr, s2);
  if (!found) {
    *pos = -1;
    return eok();
  }
  *pos = found - s->ptr;
  return eok();
}

error str_replace_all_(struct str *const s, char const *const find, char const *const replacement MEM_FILEPOS_PARAMS) {
  if (!s || !find || !replacement) {
    return errg(err_invalid_arugment);
  }
  if (!s->ptr || !s->len) {
    return eok();
  }
  ptrdiff_t const findlen = (ptrdiff_t)strlen(find);
  if (findlen == 0) {
    return eok();
  }
  error err = eok();
  struct str tmp = {0};
  ptrdiff_t pos = 0;
  for (;;) {
    char const *const found = strstr(s->ptr + pos, find);
    if (!found) {
      err = str_cat_(&tmp, s->ptr + pos MEM_FILEPOS_VALUES_PASSTHRU);
      if (efailed(err)) {
        err = ethru(err);
        goto cleanup;
      }
      break;
    }
    ptrdiff_t const foundpos = found - s->ptr;
    err = str_ncat_(&tmp, s->ptr + pos, (size_t)(foundpos - pos) MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    err = str_cat_(&tmp, replacement MEM_FILEPOS_VALUES_PASSTHRU);
    if (efailed(err)) {
      err = ethru(err);
      goto cleanup;
    }
    pos = foundpos + findlen;
  }

  err = str_cpy_(s, tmp.ptr MEM_FILEPOS_VALUES_PASSTHRU);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }

cleanup:
  array_free_core_((struct array *)&tmp MEM_FILEPOS_VALUES_PASSTHRU);
  return err;
}

#endif // USE_STR
