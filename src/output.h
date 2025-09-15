#pragma once

#include <ovbase.h>

/**
 * @brief Output string to stderr or custom hook (internal function)
 *
 * Outputs a string followed by a newline to stderr. If a custom output hook
 * has been set via ov_error_set_output_hook(), the string is sent to that
 * hook instead. On Windows, handles UTF-8 encoding properly for console output.
 *
 * @param str String to output. Must not be NULL.
 *
 * @note This is an internal function used by error reporting system.
 *       For application use, consider using standard printf/fprintf functions.
 * @note On Windows, automatically converts UTF-8 to wide characters for console output
 */
void output(char const *const str);
