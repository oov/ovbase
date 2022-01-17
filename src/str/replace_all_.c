#include "str.h"

#ifdef USE_STR

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

#endif
