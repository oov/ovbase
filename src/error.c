#include "mem.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include <ovarray.h>
#include <ovprintf_ex.h>

void ov_error_destroy(struct ov_error *const target MEM_FILEPOS_PARAMS) {
  assert(target != NULL && "target must not be NULL");
  if (!target) {
    return;
  }
  assert(target->stack[0].info.type != ov_error_type_invalid && "error is already destroyed or not initialized");
  for (size_t i = 0; i < sizeof(target->stack) / sizeof(target->stack[0]); i++) {
    if (target->stack[i].info.context) {
      if (target->stack[i].info.flag_context_is_static) {
        target->stack[i].info.context = NULL;
      } else {
        ov_array_destroy((void **)ov_deconster_(&target->stack[i].info.context) MEM_FILEPOS_VALUES_PASSTHRU);
      }
    }
    target->stack[i] = (struct ov_error_stack){0};
  }
  if (target->stack_extended) {
    size_t const ext_count = OV_ARRAY_LENGTH(target->stack_extended);
    for (size_t i = 0; i < ext_count; i++) {
      if (target->stack_extended[i].info.context) {
        if (target->stack_extended[i].info.flag_context_is_static) {
          target->stack_extended[i].info.context = NULL;
        } else {
          ov_array_destroy((void **)ov_deconster_(&target->stack_extended[i].info.context) MEM_FILEPOS_VALUES_PASSTHRU);
        }
      }
      target->stack_extended[i] = (struct ov_error_stack){0};
    }
    ov_array_destroy((void **)&target->stack_extended MEM_FILEPOS_VALUES_PASSTHRU);
  }
}

/**
 * @brief Add an error entry to the error stack (internal function)
 *
 * Adds a new error entry to the error stack. If the fixed stack is full,
 * automatically allocates and uses the extended stack. This is an internal
 * helper function used by the public error setting functions.
 *
 * @param target Target error structure to add entry to. Must not be NULL.
 * @param info Error information to add. Must not be NULL.
 * @param err Error structure for reporting failures. Can be NULL.
 * @param filepos File position information. Must not be NULL.
 * @return true on success, false on failure (check err for details)
 *
 * @note This is an internal function not intended for direct use.
 *       Use public macros like OV_ERROR_SET, OV_ERROR_PUSH instead.
 */
static bool push(struct ov_error *const target,
                 struct ov_error_info const *const info,
                 struct ov_error *const err ERR_FILEPOS_PARAMS) {
  assert(target != NULL && "target must not be NULL");
  assert(info != NULL && "info must not be NULL");
  assert(filepos != NULL && "filepos must not be NULL");
  if (!target || !info || !filepos) {
    if (err) {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    }
    return false;
  }

  struct ov_error_stack *s = NULL;

  if (OV_ARRAY_LENGTH(target->stack_extended) == 0) {
    // If call_stack_extended is not created yet, we need to find an empty slot in call_stack
    size_t const call_stack_size = sizeof(target->stack) / sizeof(target->stack[0]);
    for (size_t i = 0; i < call_stack_size; i++) {
      if (target->stack[i].info.type != ov_error_type_invalid) {
        continue;
      }
      s = &target->stack[i];
      goto cleanup;
    }
  }
  {
    // call_stack is full, we need to create or grow call_stack_extended
    size_t const len = OV_ARRAY_LENGTH(target->stack_extended);
    size_t const cap = OV_ARRAY_CAPACITY(target->stack_extended);
    if (len >= cap) {
      if (!ov_array_grow(
              (void **)&target->stack_extended, sizeof(struct ov_error_stack), cap + 4 MEM_FILEPOS_VALUES_PASSTHRU)) {
        OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
        goto cleanup;
      }
    }
    OV_ARRAY_SET_LENGTH(target->stack_extended, len + 1);
    s = &target->stack_extended[len];
  }
cleanup:
  if (s) {
    *s = (struct ov_error_stack){
        .info = *info,
        .filepos = *filepos,
    };
  }
  return s;
}

/**
 * @brief Add formatted error entry to the error stack (internal function)
 *
 * Formats an error message using printf-style formatting and adds it to the error stack.
 * The reference parameter provides translation safety when the format string comes from
 * external sources like gettext. This ensures that translated format strings are
 * compatible with the original argument types and order.
 *
 * @param target Target error structure to add entry to. Must not be NULL.
 * @param info Error information containing format string in context field. Must not be NULL.
 *             The info->context must contain the format string for printf-style formatting.
 * @param reference Reference format string for translation safety. Can be NULL.
 *                  When non-NULL, provides a reference pattern to validate format compatibility.
 * @param err Error structure for reporting failures. Can be NULL.
 * @param filepos File position information. Must not be NULL.
 * @param valist Variable argument list containing format arguments.
 * @return true on success, false on failure (check err for details)
 *
 * @note This is an internal function not intended for direct use.
 *       Use public macros like OV_ERROR_SETF, OV_ERROR_PUSHF instead.
 * @note The reference parameter works the same way as in ovprintf.h functions,
 *       providing translation safety for internationalized error messages.
 *
 * @example Internal usage (via public macro):
 *   OV_ERROR_SETF(&err, ov_error_type_generic, ov_error_generic_fail,
 *                 "%1$s%2$d", gettext("File %1$s not found, error %2$d"), filename, errno);
 */
static bool pushfv(struct ov_error *const target,
                   struct ov_error_info const *const info,
                   char const *const reference,
                   struct ov_error *const err ERR_FILEPOS_PARAMS,
                   va_list valist) {
  assert(target != NULL && "target must not be NULL");
  assert(info != NULL && "info must not be NULL");
  assert(info->context != NULL && "Format string is required for formatted error");
  assert(filepos != NULL && "filepos must not be NULL");
  if (!target || !info || !info->context || !filepos) {
    if (err) {
      OV_ERROR_SET_GENERIC(err, ov_error_generic_invalid_argument);
    }
    return false;
  }

  char *context = NULL;
  bool result = false;

  bool const b = ov_vsprintf_char(&context, reference, info->context, valist);
  if (!b) {
    OV_ERROR_SET_GENERIC(err, ov_error_generic_fail);
    goto cleanup;
  }
  if (!push(
          target, &(struct ov_error_info const){info->type, 0, info->code, context}, err ERR_FILEPOS_VALUES_PASSTHRU)) {
    OV_ERROR_ADD_TRACE(err);
    goto cleanup;
  }
  result = true;

cleanup:
  return result;
}

void ov_error_set(struct ov_error *const target, struct ov_error_info const *const info ERR_FILEPOS_PARAMS) {
  if (!target) {
    return; // accept NULL target, do nothing
  }
  assert(info != NULL && "info must not be NULL");
  assert(filepos != NULL && "filepos must not be NULL");
  assert(info->type != ov_error_type_invalid && "error type must be valid");
  assert(target->stack[0].info.type == ov_error_type_invalid && "error is already set - use OV_ERROR_DESTROY first");
  if (!info || !filepos || info->type == ov_error_type_invalid) {
    return; // invalid parameters
  }
  if (target->stack[0].info.type != ov_error_type_invalid) {
    ov_error_destroy(target MEM_FILEPOS_VALUES_PASSTHRU);
  }
  target->stack[0] = (struct ov_error_stack){
      .filepos = *filepos,
      .info = *info,
  };
#if defined(LEAK_DETECTOR) || defined(ALLOCATE_LOGGER)
  {
    // Force heap allocation to ensure leak detection works properly
    bool const r =
        ov_array_grow((void **)&target->stack_extended, sizeof(struct ov_error_stack), 4 MEM_FILEPOS_VALUES_PASSTHRU);
#  ifdef NDEBUG
    (void)r;
#  else
    assert(r && "memory allocation for stack_extended should not fail");
#  endif
  }
#endif
}

void ov_error_setf(struct ov_error *const target,
                   struct ov_error_info const *const info,
                   char const *const reference ERR_FILEPOS_PARAMS,
                   ...) {
  if (!target) {
    return; // accept NULL target, do nothing
  }
  assert(info != NULL && "info must not be NULL");
  assert(filepos != NULL && "filepos must not be NULL");
  assert(info->context != NULL && "Format string is required for ov_error_setf");
  assert(target->stack[0].info.type == ov_error_type_invalid && "error is already set - use OV_ERROR_DESTROY first");
  if (!info || !filepos || !info->context) {
    return; // invalid parameters
  }
  if (target->stack[0].info.type != ov_error_type_invalid) {
    ov_error_destroy(target MEM_FILEPOS_VALUES_PASSTHRU);
  }

  va_list valist;
  va_start(valist, filepos);
  struct ov_error local_err = {0};
  bool success = pushfv(target, info, reference, &local_err ERR_FILEPOS_VALUES_PASSTHRU, valist);
  va_end(valist);

  if (!success) {
    // If pushfv failed, set basic error info using safe method
    ov_error_set(target, &(struct ov_error_info){info->type, 0, info->code, NULL} ERR_FILEPOS_VALUES_PASSTHRU);
    OV_ERROR_DESTROY(&local_err);
  }
}

void ov_error_push(struct ov_error *const target, struct ov_error_info const *const info ERR_FILEPOS_PARAMS) {
  assert(filepos != NULL && "filepos must not be NULL");
  if (!target) {
    return; // accept NULL target
  }
  if (!filepos) {
    return; // invalid parameters, do nothing
  }
  if (target->stack[0].info.type == ov_error_type_invalid) {
    return; // Not in error state
  }
  if (!push(target,
            info ? info : &(struct ov_error_info){ov_error_type_generic, 0, ov_error_generic_trace, NULL},
            NULL ERR_FILEPOS_VALUES_PASSTHRU)) {
    // TODO: report error
    return;
  }
}

void ov_error_pushf(struct ov_error *const target,
                    struct ov_error_info const *const info,
                    char const *const reference ERR_FILEPOS_PARAMS,
                    ...) {
  assert(info != NULL && "info must not be NULL");
  assert(filepos != NULL && "filepos must not be NULL");
  if (!target) {
    return; // accept NULL target
  }
  if (!info || !filepos || !info->context) {
    return; // invalid parameters, do nothing
  }
  assert(info->context != NULL && "Format string is required for ov_error_pushf");
  if (target->stack[0].info.type == ov_error_type_invalid) {
    return; // Not in error state
  }

  va_list valist;
  va_start(valist, filepos);
  struct ov_error local_err = {0};
  bool success = pushfv(target, info, reference, &local_err ERR_FILEPOS_VALUES_PASSTHRU, valist);
  va_end(valist);

  if (!success) {
    // TODO: report error
    OV_ERROR_DESTROY(&local_err);
  }
}

static size_t
find_entry(struct ov_error_stack const *const entries, size_t const length, int const type, int const code) {
  assert(entries != NULL && "entries must not be NULL");
  for (size_t i = 0; i < length; i++) {
    if (entries[i].info.type == ov_error_type_invalid) {
      break; // Stop when we hit invalid entry
    }
    if (entries[i].info.type == type && entries[i].info.code == code) {
      return i;
    }
  }
  return SIZE_MAX;
}

bool ov_error_is(struct ov_error const *const target, int const type, int const code) {
  assert(target != NULL && "target must not be NULL");
  // Return true if target is NULL to ensure logically broken code fails explicitly
  // rather than silently appearing to work correctly in release builds
  if (target == NULL) {
    return true;
  }
  size_t const fixed_stack_size = sizeof(target->stack) / sizeof(target->stack[0]);
  if (find_entry(target->stack, fixed_stack_size, type, code) != SIZE_MAX) {
    return true;
  }
  size_t const ext_count = OV_ARRAY_LENGTH(target->stack_extended);
  if (find_entry(target->stack_extended, ext_count, type, code) != SIZE_MAX) {
    return true;
  }
  return false;
}

static size_t find_entry_by_type(struct ov_error_stack const *const entries, size_t const length, int const type) {
  assert(entries != NULL && "entries must not be NULL");
  for (size_t i = 0; i < length; i++) {
    if (entries[i].info.type == ov_error_type_invalid) {
      break; // Stop when we hit invalid entry
    }
    if (entries[i].info.type == type) {
      return i;
    }
  }
  return SIZE_MAX;
}

bool ov_error_get_code(struct ov_error const *const target, int const type, int *const code) {
  assert(target != NULL && "target must not be NULL");
  assert(code != NULL && "code must not be NULL");

  if (!target || !code) {
    return false;
  }

  size_t const fixed_stack_size = sizeof(target->stack) / sizeof(target->stack[0]);
  size_t entry_idx = find_entry_by_type(target->stack, fixed_stack_size, type);
  if (entry_idx != SIZE_MAX) {
    *code = target->stack[entry_idx].info.code;
    return true;
  }

  size_t const ext_count = OV_ARRAY_LENGTH(target->stack_extended);
  entry_idx = find_entry_by_type(target->stack_extended, ext_count, type);
  if (entry_idx != SIZE_MAX) {
    *code = target->stack_extended[entry_idx].info.code;
    return true;
  }

  return false; // Not found
}
