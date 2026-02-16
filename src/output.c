#include "output.h"

#include <assert.h>

static ov_error_output_hook_func g_output_hook = NULL;

void ov_error_set_output_hook(ov_error_output_hook_func hook_func) { g_output_hook = hook_func; }

void output(enum ov_error_severity severity, char const *const str) {
  assert(str != NULL && "str must not be NULL");
  if (!str) {
    return;
  }

  if (!g_output_hook) {
    return;
  }

  g_output_hook(severity, str);
}
