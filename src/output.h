#pragma once

#include <ovbase.h>

/**
 * @brief Set the output hook function (internal)
 *
 * Called from ov_init() to configure the output hook.
 *
 * @param hook_func Function to handle error output, or NULL to disable output
 */
void ov_error_set_output_hook(ov_error_output_hook_func hook_func);

/**
 * @brief Set the autofill hook function (internal)
 *
 * Called from ov_init() to configure the autofill hook.
 *
 * @param hook_func Hook function pointer, or NULL to disable custom hook
 */
void ov_error_set_autofill_hook(ov_error_autofill_hook_func hook_func);

/**
 * @brief Output string via the registered output hook (internal function)
 *
 * Routes a string to the output hook set via ov_init().
 * If no hook is set, the message is silently discarded.
 *
 * @param severity Error severity level
 * @param str String to output. Must not be NULL.
 *
 * @note This is an internal function used by error reporting system.
 */
void output(enum ov_error_severity severity, char const *const str);
