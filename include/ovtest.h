#pragma once

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include <ovarray.h>
#include <ovbase.h>

#ifdef __GNUC__

#  pragma GCC diagnostic push
#  if __has_warning("-Wused-but-marked-unused")
#    pragma GCC diagnostic ignored "-Wused-but-marked-unused"
#  endif

#endif // __GNUC__

static inline void test_init_(void) {
  setlocale(LC_ALL, "");
  {
    struct ov_init_options opts = ov_init_get_default_options();
    if (!ov_init(&opts)) {
      abort();
    }
  }
#ifdef TEST_MY_INIT
  TEST_MY_INIT;
#endif
}

static inline void test_fini_(void) {
#ifdef TEST_MY_FINI
  TEST_MY_FINI;
#endif
  ov_exit();
#ifdef LEAK_DETECTOR
  if (ov_mem_get_allocated_count()) {
    printf("!! MEMORY LEAKED !!\n");
    abort();
  }
#endif
}
#define TEST_INIT test_init_()
#define TEST_FINI test_fini_()

#ifdef __GNUC__

#  pragma GCC diagnostic push
#  if __has_warning("-Wsign-conversion")
#    pragma GCC diagnostic ignored "-Wsign-conversion"
#  endif
#  if __has_warning("-Wmissing-prototypes")
#    pragma GCC diagnostic ignored "-Wmissing-prototypes"
#  endif
#  if __has_warning("-Wimplicit-int-float-conversion")
#    pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"
#  endif
#  if __has_warning("-Wdisabled-macro-expansion")
#    pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#  endif
#  if __has_warning("-Wunused-parameter")
#    pragma GCC diagnostic ignored "-Wunused-parameter"
#  endif
#  if __has_warning("-Wunused-function")
#    pragma GCC diagnostic ignored "-Wunused-function"
#  endif
#  if __has_warning("-Wswitch-enum")
#    pragma GCC diagnostic ignored "-Wswitch-enum"
#  endif
#  if __has_warning("-Wswitch-default")
#    pragma GCC diagnostic ignored "-Wswitch-default"
#  endif
#  if __has_warning("-Wformat")
#    pragma GCC diagnostic ignored "-Wformat"
#  endif
#  include <ovbase_3rd/acutest.h>
#  pragma GCC diagnostic pop

#else

#  include <ovbase_3rd/acutest.h>

#endif // __GNUC__

#ifdef __EMSCRIPTEN__
/* Emscripten does not support fork(); force acutest to run tests in-process. */
__attribute__((constructor)) static void ovtest_emscripten_no_exec_(void) { acutest_no_exec_ = 1; }
#endif

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif // __GNUC__

static inline bool ovtest_should_run_benchmarks(void) {
  char const *env = getenv("OVTEST_RUN_BENCHMARKS");
  if (env && env[0] == '1' && env[1] == '\0') {
    return true;
  }
  TEST_SKIP("Skipping benchmark tests. Set OVTEST_RUN_BENCHMARKS=1 environment variable to run.");
  return false;
}

/**
 * @brief Check that an operation succeeded, output error message if failed, and clean up error
 *
 * Combines TEST_CHECK with automatic error message output and cleanup on failure.
 * This is the recommended way to check operations that use struct ov_error.
 * The error is only destroyed if the operation fails and an error is set.
 * Returns the result so it can be used in conditionals.
 *
 * @param result Boolean result of the operation (true = success)
 * @param err_ptr Pointer to struct ov_error that may contain error info
 * @return Non-zero if condition passes, 0 if it fails
 *
 * @example
 *   struct ov_error err = {0};
 *   if (!TEST_SUCCEEDED(some_function(&err), &err)) {
 *     // err is already destroyed on failure
 *     return;
 *   }
 */
#define TEST_SUCCEEDED(result, err_ptr) ovtest_succeeded_((result), (err_ptr), __FILE__, __LINE__, #result)

/**
 * @brief Assert that an operation succeeded, abort test if failed
 *
 * Similar to TEST_SUCCEEDED but aborts the current test if the operation fails.
 * The error is only destroyed if the operation fails and an error is set.
 * Use with caution as it may cause resource leaks (see TEST_ASSERT in acutest.h).
 *
 * @param result Boolean result of the operation (true = success)
 * @param err_ptr Pointer to struct ov_error that may contain error info
 *
 * @example
 *   struct ov_error err = {0};
 *   TEST_ASSERT_SUCCEEDED(critical_init(&err), &err);
 *   // If we reach here, initialization succeeded
 */
#define TEST_ASSERT_SUCCEEDED(result, err_ptr)                                                                         \
  do {                                                                                                                 \
    if (!ovtest_succeeded_((result), (err_ptr), __FILE__, __LINE__, #result))                                          \
      acutest_abort_();                                                                                                \
  } while (0)

/**
 * @brief Check that an operation failed with a specific error type and code
 *
 * Verifies that the operation returned false and the error matches the expected type and code.
 * The error is always destroyed after checking.
 * Returns non-zero if the operation failed with the expected error, 0 otherwise.
 *
 * @param result Boolean result of the operation (expected to be false)
 * @param err_ptr Pointer to struct ov_error that should contain the expected error
 * @param error_type Expected error type (e.g., ov_error_type_generic)
 * @param error_code Expected error code (e.g., ov_error_generic_invalid_argument)
 * @return Non-zero if operation failed with expected error, 0 otherwise
 *
 * @example
 *   struct ov_error err = {0};
 *   TEST_FAILED_WITH(some_function(NULL, &err), &err, ov_error_type_generic, ov_error_generic_invalid_argument);
 */
#define TEST_FAILED_WITH(result, err_ptr, error_type, error_code)                                                      \
  ovtest_failed_with_((result), (err_ptr), (error_type), (error_code), __FILE__, __LINE__, #result)

static inline void ovtest_error_to_msg_(struct ov_error const *const err) {
  char *msg = NULL;
  struct ov_error conv_err = {0};
  if (ov_error_to_string(err, &msg, true, &conv_err)) {
    TEST_MSG("Error: %s", msg);
    goto cleanup;
  }
  if (ov_error_to_string(&conv_err, &msg, true, NULL)) {
    TEST_MSG("Error: (failed to convert error to string: %s)", msg);
    goto cleanup;
  }
  TEST_MSG("Error: (failed to convert error to string)");
cleanup:
  if (msg) {
    OV_ARRAY_DESTROY(&msg);
  }
}

static inline int ovtest_succeeded_(
    bool const result, struct ov_error *const err, char const *const file, int const line, char const *const expr) {
  int const ret = acutest_check_(result && err, file, line, "%s", expr);
  if (ret) {
    return ret;
  }
  if (!err) {
    TEST_MSG("err CANNOT be NULL");
    return ret;
  }
  if (!result) {
    ovtest_error_to_msg_(err);
    OV_ERROR_DESTROY(err);
  }
  return ret;
}

static inline int ovtest_failed_with_(bool const result,
                                      struct ov_error *const err,
                                      int const expected_type,
                                      int const expected_code,
                                      char const *const file,
                                      int const line,
                                      char const *const expr) {
  int const ret = acutest_check_(
      !result && err && ov_error_is(err, expected_type, expected_code), file, line, "!(%s) with expected error", expr);
  if (ret) {
    OV_ERROR_DESTROY(err);
    return ret;
  }
  if (!err) {
    TEST_MSG("err CANNOT be NULL");
    return ret;
  }
  if (result) {
    TEST_MSG("want failure, got success");
    return ret;
  }
  if (!ov_error_is(err, expected_type, expected_code)) {
    TEST_MSG("want type=%d code=%d, got type=%d code=%d",
             expected_type,
             expected_code,
             err->stack[0].info.type,
             err->stack[0].info.code);
  }
  OV_ERROR_DESTROY(err);
  return ret;
}
