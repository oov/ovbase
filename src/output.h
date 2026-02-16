#pragma once

#include <ovbase.h>

/**
 * @brief Output string via the registered output hook (internal function)
 *
 * Routes a string to the output hook set via ov_error_set_output_hook() or ov_init().
 * If no hook is set, the message is silently discarded.
 * Messages are filtered by severity level based on the OV_ERROR_LEVEL environment variable.
 *
 * @param severity Error severity level
 * @param str String to output. Must not be NULL.
 *
 * @note This is an internal function used by error reporting system.
 */
void output(enum ov_error_severity severity, char const *const str);
