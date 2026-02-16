#include "output.h"

#include <assert.h>
#include <ovnum.h>
#include <stdlib.h>

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

  static int limit = -1;
  if (limit == -1) {
    char const *const env = getenv("OV_ERROR_LEVEL");
    int64_t v = 0;
    if (env && ov_atoi_char(env, &v, false)) {
      limit = (int)v;
    } else {
      limit = ov_error_severity_info;
    }
  }

  if ((int)severity > limit) {
    return;
  }

  g_output_hook(severity, str);
}
