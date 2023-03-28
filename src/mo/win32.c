#include "ovmo.h"

#include "ovprintf.h"

#ifdef _WIN32
NODISCARD error mo_parse_from_resource(struct mo **const mpp, HINSTANCE const hinst, uint16_t const lang_id) {
  HRSRC r = FindResourceExW(hinst, MAKEINTRESOURCEW(10), L"MO", lang_id);
  if (r == NULL) {
    return 0;
  }
  HGLOBAL h = LoadResource(hinst, r);
  if (!h) {
    return 0;
  }
  void const *const p = LockResource(h);
  if (!p) {
    return 0;
  }
  size_t const sz = (size_t)(SizeofResource(hinst, r));
  if (!sz) {
    return 0;
  }
  error err = mo_parse(mpp, p, sz);
  if (efailed(err)) {
    err = ethru(err);
    goto cleanup;
  }
cleanup:
  return err;
}
#endif
