#include <ovtest.h>

#include <ovbase.h>

// Helper functions to reduce code duplication
static void
verify_error_state(struct ov_error *err, int expected_type, int expected_code, const char *expected_context) {
  TEST_CHECK(err->stack[0].info.type == expected_type);
  TEST_CHECK(err->stack[0].info.code == expected_code);
  if (expected_context) {
    TEST_CHECK(err->stack[0].info.context != NULL);
    TEST_CHECK(strstr(err->stack[0].info.context, expected_context) != NULL);
  } else {
    TEST_CHECK(err->stack[0].info.context == NULL);
  }
}

static void verify_clean_state(struct ov_error *err) {
  TEST_CHECK(err->stack[0].info.type == ov_error_type_invalid);
  TEST_CHECK(err->stack[0].info.code == 0);
  TEST_CHECK(err->stack[0].info.context == NULL);
}

static void test_and_cleanup(struct ov_error *err, int expected_type, int expected_code, const char *expected_context) {
  verify_error_state(err, expected_type, expected_code, expected_context);
  OV_ERROR_DESTROY(err);
  verify_clean_state(err);
}

static void test_ov_error_core_functionality(void) {
  struct ov_error err = {0};

  // Test initialization
  verify_clean_state(&err);

  // Test basic error setting with position tracking
  OV_ERROR_SET_GENERIC(&err, ov_error_generic_fail);
  verify_error_state(&err, ov_error_type_generic, ov_error_generic_fail, NULL);
  TEST_CHECK(err.stack[0].filepos.file != NULL);
  TEST_CHECK(err.stack[0].filepos.func != NULL);
  TEST_CHECK(err.stack[0].filepos.line > 0);
  OV_ERROR_DESTROY(&err);
  verify_clean_state(&err);

  // Test formatted error and proper error replacement
  OV_ERROR_SETF(&err, ov_error_type_generic, ov_error_generic_invalid_argument, NULL, "%1$s", "Test message");
  verify_error_state(&err, ov_error_type_generic, ov_error_generic_invalid_argument, "Test message");

  // Proper way to replace an error: destroy first, then set new one
  OV_ERROR_DESTROY(&err);
  OV_ERROR_SET_GENERIC(&err, ov_error_generic_fail);
  verify_error_state(&err, ov_error_type_generic, ov_error_generic_fail, NULL);

  OV_ERROR_DESTROY(&err);
  verify_clean_state(&err);
}

static void test_ov_error_null_safety(void) {
  // Test that all operations handle NULL pointers safely without crashing

  // Should not crash when err is NULL for OV_ERROR_SET_*
  OV_ERROR_SET_GENERIC(NULL, ov_error_generic_fail);
  OV_ERROR_SET_ERRNO(NULL, EINVAL);
#ifdef _WIN32
  OV_ERROR_SET_HRESULT(NULL, E_FAIL);
#endif

  // Should not crash when err is NULL for OV_ERROR_DESTROY
  OV_ERROR_DESTROY(NULL);

  // Should not crash when err is NULL for OV_ERROR_TRACE (tested later in stack tests)
  OV_ERROR_TRACE(NULL);

  // All tests pass if we don't crash
}

static void test_ov_error_different_types(void) {
  struct ov_error err = {0};

  // Test all error types with helper function
  OV_ERROR_SET_GENERIC(&err, ov_error_generic_out_of_memory);
  test_and_cleanup(&err, ov_error_type_generic, ov_error_generic_out_of_memory, NULL);

  OV_ERROR_SET_ERRNO(&err, EINVAL);
  test_and_cleanup(&err, ov_error_type_errno, EINVAL, NULL);

#ifdef _WIN32
  OV_ERROR_SET_HRESULT(&err, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
  test_and_cleanup(&err, ov_error_type_hresult, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), NULL);
#endif
}

static void test_ov_error_set_macro(void) {
  struct ov_error err = {0};

  // Test that OV_ERROR_SET creates proper error structures
  OV_ERROR_SET(&err, ov_error_type_generic, ov_error_generic_fail, "This is a test error message");
  TEST_CHECK(err.stack[0].info.type == ov_error_type_generic);
  TEST_CHECK(err.stack[0].info.code == ov_error_generic_fail);
  TEST_CHECK(strcmp(err.stack[0].info.context, "This is a test error message") == 0);

  OV_ERROR_DESTROY(&err);
}

static void test_ov_error_filepos_tracking(void) {
  struct ov_error err = {0};

  OV_ERROR_SET_GENERIC(&err, ov_error_generic_fail);

  TEST_CHECK(err.stack[0].info.type == ov_error_type_generic);
  TEST_CHECK(err.stack[0].info.code == ov_error_generic_fail);

  // Check that file position is captured
  TEST_CHECK(err.stack[0].filepos.file != NULL);
  TEST_CHECK(err.stack[0].filepos.func != NULL);
  TEST_CHECK(err.stack[0].filepos.line > 0);

  // Check that it's this function
  TEST_CHECK(strstr(err.stack[0].filepos.func, "test_ov_error_filepos_tracking") != NULL);

  OV_ERROR_DESTROY(&err);
}

static void test_ov_error_lifecycle_management(void) {
  struct ov_error err = {0};

  // Test success state (still creates error state with type set)
  OV_ERROR_SET_GENERIC(&err, ov_error_generic_success);
  verify_error_state(&err, ov_error_type_generic, ov_error_generic_success, NULL);
  OV_ERROR_DESTROY(&err);

  // Test structure reuse
  verify_clean_state(&err);
  OV_ERROR_SETF(&err, ov_error_type_generic, ov_error_generic_fail, NULL, "%s", "Test message");
  test_and_cleanup(&err, ov_error_type_generic, ov_error_generic_fail, "Test message");

  // Reuse with different error type
  OV_ERROR_SET_ERRNO(&err, ENOENT);
  test_and_cleanup(&err, ov_error_type_errno, ENOENT, NULL);
}

// Helper functions for call stack tests
static void level3_function(struct ov_error *err) { OV_ERROR_SET_GENERIC(err, ov_error_generic_fail); }

static void level2_function(struct ov_error *err) {
  level3_function(err);
  if (err->stack[0].info.type != ov_error_type_invalid) {
    OV_ERROR_TRACE(err);
  }
}

static void level1_function(struct ov_error *err) {
  level2_function(err);
  if (err->stack[0].info.type != ov_error_type_invalid) {
    OV_ERROR_TRACE(err);
  }
}

static void test_ov_error_push_basic(void) {
  struct ov_error err = {0};

  // Test basic push and edge cases
  OV_ERROR_SET_GENERIC(&err, ov_error_generic_fail);
  verify_error_state(&err, ov_error_type_generic, ov_error_generic_fail, NULL);
  TEST_CHECK(err.stack[0].filepos.file != NULL);
  TEST_CHECK(err.stack[1].filepos.file == NULL);

  OV_ERROR_TRACE(&err);
  TEST_CHECK(err.stack[1].filepos.file != NULL);
  TEST_CHECK(err.stack[2].filepos.file == NULL);

  OV_ERROR_DESTROY(&err);

  // Test NULL safety and non-error state
  OV_ERROR_TRACE(NULL); // Should not crash
  verify_clean_state(&err);
  OV_ERROR_TRACE(&err); // Should not modify clean state
  verify_clean_state(&err);
}

static void test_ov_error_push_multiple_levels(void) {
  struct ov_error err = {0};

  level1_function(&err);
  TEST_CHECK(err.stack[0].info.type != ov_error_type_invalid);

  // Check that we have multiple levels in call stack
  TEST_CHECK(err.stack[0].filepos.file != NULL); // Original error
  TEST_CHECK(err.stack[1].filepos.file != NULL); // level2_function
  TEST_CHECK(err.stack[2].filepos.file != NULL); // level1_function

  // Check function names (if available)
  TEST_CHECK(strstr(err.stack[0].filepos.func, "level3_function") != NULL);
  TEST_CHECK(strstr(err.stack[1].filepos.func, "level2_function") != NULL);
  TEST_CHECK(strstr(err.stack[2].filepos.func, "level1_function") != NULL);

  OV_ERROR_DESTROY(&err);
}

static void test_ov_error_push_call_stack_overflow(void) {
  struct ov_error err = {0};

  OV_ERROR_SET_GENERIC(&err, ov_error_generic_fail);
  TEST_CHECK(err.stack[0].info.type == ov_error_type_generic);
  TEST_CHECK(err.stack[0].info.code == ov_error_generic_fail);

  // Fill up the call_stack array (first one already used)
  size_t const stack_size = sizeof(err.stack) / sizeof(err.stack[0]);
  for (size_t i = 1; i < stack_size; i++) {
    OV_ERROR_TRACE(&err);
    TEST_CHECK(err.stack[i].filepos.file != NULL);
  }

  // All call_stack slots should be filled
  for (size_t i = 0; i < stack_size; i++) {
    TEST_CHECK(err.stack[i].filepos.file != NULL);
  }
#if defined(LEAK_DETECTOR) || defined(ALLOCATE_LOGGER)
  // When leak detection is enabled, stack_extended is always allocated for tracking
  TEST_CHECK(err.stack_extended != NULL);
  TEST_CHECK(OV_ARRAY_LENGTH(err.stack_extended) == 0);
#else
  TEST_CHECK(err.stack_extended == NULL);
#endif

  // Next push should go to call_stack_extended
  OV_ERROR_TRACE(&err);
  TEST_CHECK(err.stack_extended != NULL);
  TEST_CHECK(OV_ARRAY_LENGTH(err.stack_extended) == 1);

  // Add one more to extended with formatted message (heap allocation)
  OV_ERROR_TRACEF(&err, NULL, "Extended stack trace with number %d", 42);
  TEST_CHECK(OV_ARRAY_LENGTH(err.stack_extended) == 2);
  TEST_CHECK(err.stack_extended[1].info.context != NULL);
  TEST_CHECK(strstr(err.stack_extended[1].info.context, "42") != NULL);

  OV_ERROR_DESTROY(&err);
  TEST_CHECK(err.stack_extended == NULL);
}

static void test_ov_error_formatting(void) {
  struct ov_error err = {0};

  // Test various formatting scenarios with helper functions
  OV_ERROR_SET_GENERIC(&err, ov_error_generic_fail);
  test_and_cleanup(&err, ov_error_type_generic, ov_error_generic_fail, NULL);

  // Basic formatting
  OV_ERROR_SETF(&err,
                ov_error_type_generic,
                ov_error_generic_fail,
                "%1$s %2$d",
                "File %s not found, error code %d",
                "test.txt",
                404);
  test_and_cleanup(&err, ov_error_type_generic, ov_error_generic_fail, "test.txt");

  // Complex formatting with multiple types
  OV_ERROR_SETF(&err,
                ov_error_type_generic,
                ov_error_generic_invalid_argument,
                "%1$s %2$d %3$f %4$c",
                "Invalid: %s %d %f %c",
                "buffer",
                5,
                3.14,
                'X');
  verify_error_state(&err, ov_error_type_generic, ov_error_generic_invalid_argument, "buffer");
  TEST_CHECK(strstr(err.stack[0].info.context, "5") != NULL);
  TEST_CHECK(strstr(err.stack[0].info.context, "3.14") != NULL);
  TEST_CHECK(strstr(err.stack[0].info.context, "X") != NULL);
  OV_ERROR_DESTROY(&err);

  // Different error types with formatting
  OV_ERROR_SETF(&err, ov_error_type_errno, ENOENT, NULL, "%s", "File not found");
  test_and_cleanup(&err, ov_error_type_errno, ENOENT, "File not found");

#ifdef _WIN32
  OV_ERROR_SETF(&err,
                ov_error_type_hresult,
                (int)0x80070002L,
                "%1$s %2$x",
                "Error: %s, code: 0x%x",
                "Access denied",
                0x80070002L);
  test_and_cleanup(&err, ov_error_type_hresult, (int)0x80070002L, "Access denied");
#endif
}

static void test_ov_error_direct_parameters(void) {
  struct ov_error err = {0};

  // Test error setting with direct parameters
  OV_ERROR_SET(&err, ov_error_type_generic, ov_error_generic_fail, "Test error message 1");

  // Use public API to verify error state
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_fail));

  // Test string conversion to verify message
  char *str = NULL;
  TEST_CHECK(ov_error_to_string(&err, &str, false, NULL));
  TEST_CHECK(str != NULL);
  TEST_CHECK(strstr(str, "Test error message 1") != NULL);
  OV_ARRAY_DESTROY(&str);

  OV_ERROR_DESTROY(&err);

  // Test with NULL message
  OV_ERROR_SET(&err, ov_error_type_generic, ov_error_generic_fail, NULL);
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_fail));

  OV_ERROR_DESTROY(&err);
}

static void test_ov_error_message_autofill(void) {
  struct ov_error_stack target = {0};

  // Test autofill for error without existing message
  target.info.type = ov_error_type_generic;
  target.info.code = ov_error_generic_fail;
  target.info.context = NULL;

  TEST_CHECK(ov_error_autofill_message(&target, NULL));
  TEST_CHECK(target.info.context != NULL);
  TEST_CHECK(strcmp(target.info.context, "operation failed") == 0);

  if (target.info.context) {
    if (target.info.flag_context_is_static) {
      target.info.context = NULL;
      target.info.flag_context_is_static = 0;
    } else {
      OV_ARRAY_DESTROY(ov_deconster_(&target.info.context));
    }
  }

  // Test autofill preserves existing message
  target = (struct ov_error_stack){0};
  target.info.type = ov_error_type_generic;
  target.info.code = ov_error_generic_fail;
  target.info.context = "Custom message";
  target.info.flag_context_is_static = -1;

  TEST_CHECK(ov_error_autofill_message(&target, NULL));
  TEST_CHECK(target.info.context != NULL);
  TEST_CHECK(strcmp(target.info.context, "Custom message") == 0);
}

static void test_ov_error_string_conversion(void) {
  struct ov_error target = {0};
  char *str = NULL;

  OV_ERROR_SET(&target, ov_error_type_generic, ov_error_generic_fail, "Test message");
  OV_ERROR_TRACE(&target);
  OV_ERROR_PUSH(&target, ov_error_type_generic, ov_error_generic_fail, "Test parent message");

  // Test both basic and stack trace conversion
  TEST_CHECK(ov_error_to_string(&target, &str, false, NULL));
  TEST_CHECK(str != NULL && strstr(str, "[01:0x00000001] Test message"));
  TEST_CHECK(str != NULL && !strstr(str, "  error_test.c:"));
  TEST_CHECK(str != NULL && !strstr(str, "[01:0x00000001] Test parent message"));
  OV_ARRAY_DESTROY(&str);

  TEST_CHECK(ov_error_to_string(&target, &str, true, NULL));
  TEST_CHECK(str != NULL && strstr(str, "[01:0x00000001] Test message"));
  TEST_CHECK(str != NULL && strstr(str, "  error_test.c:"));
  TEST_CHECK(str != NULL && strstr(str, "[01:0x00000001] Test parent message"));
  OV_ARRAY_DESTROY(&str);
  OV_ERROR_DESTROY(&target);
}

// Test hook functions for autofill hook testing
static bool test_hook_success(struct ov_error_stack *target, struct ov_error *err) {
  if (target->info.type == ov_error_type_generic && target->info.code == ov_error_generic_fail) {
    // Use static string for custom message
    static char const custom_hook_msg[] = "Custom hook message";
    target->info.context = custom_hook_msg;
    target->info.flag_context_is_static = -1;
    return true;
  }
  if (target->info.type == 999 && target->info.code == 123) {
    // Use dynamic allocation for variable messages
    char *custom_msg = NULL;
    char const msg[] = "Custom error type handled by hook";
    if (!OV_ARRAY_GROW(&custom_msg, sizeof(msg), err)) {
      return false;
    }
    strcpy(custom_msg, msg);
    target->info.context = custom_msg;
    target->info.flag_context_is_static = 0;
    return true;
  }
  // No custom message - let default handling continue
  return true;
}

static bool test_hook_failure(struct ov_error_stack *target, struct ov_error *err) {
  (void)target;
  OV_ERROR_SET_GENERIC(err, ov_error_generic_out_of_memory);
  return false;
}

static void test_ov_error_autofill_hook(void) {
  struct ov_error_stack target = {0};
  struct ov_error err = {0};

  // Test with no hook (default behavior)
  ov_error_set_autofill_hook(NULL);
  target.info.type = ov_error_type_generic;
  target.info.code = ov_error_generic_fail;
  target.info.context = NULL;

  TEST_CHECK(ov_error_autofill_message(&target, NULL));
  TEST_CHECK(target.info.context != NULL);
  TEST_CHECK(strcmp(target.info.context, "operation failed") == 0);

  if (target.info.context) {
    if (target.info.flag_context_is_static) {
      target.info.context = NULL;
      target.info.flag_context_is_static = 0;
    } else {
      OV_ARRAY_DESTROY(ov_deconster_(&target.info.context));
    }
  }

  // Test hook providing custom message
  ov_error_set_autofill_hook(test_hook_success);
  target = (struct ov_error_stack){0};
  target.info.type = ov_error_type_generic;
  target.info.code = ov_error_generic_fail;
  target.info.context = NULL;

  TEST_CHECK(ov_error_autofill_message(&target, NULL));
  TEST_CHECK(target.info.context != NULL);
  TEST_CHECK(strcmp(target.info.context, "Custom hook message") == 0);

  // Test hook for unknown error type
  target = (struct ov_error_stack){0};
  target.info.type = 999;
  target.info.code = 123;
  target.info.context = NULL;

  TEST_CHECK(ov_error_autofill_message(&target, NULL));
  TEST_CHECK(target.info.context != NULL);
  TEST_CHECK(strcmp(target.info.context, "Custom error type handled by hook") == 0);

  // Cleanup dynamic message from hook
  if (target.info.context) {
    if (target.info.flag_context_is_static) {
      target.info.context = NULL;
      target.info.flag_context_is_static = 0;
    } else {
      OV_ARRAY_DESTROY(ov_deconster_(&target.info.context));
    }
  }

  // Test hook fallback to default
  target = (struct ov_error_stack){0};
  target.info.type = ov_error_type_generic;
  target.info.code = ov_error_generic_out_of_memory;
  target.info.context = NULL;

  TEST_CHECK(ov_error_autofill_message(&target, NULL));
  TEST_CHECK(target.info.context != NULL);
  TEST_CHECK(strcmp(target.info.context, "out of memory") == 0);

  if (target.info.context) {
    if (target.info.flag_context_is_static) {
      target.info.context = NULL;
      target.info.flag_context_is_static = 0;
    } else {
      OV_ARRAY_DESTROY(ov_deconster_(&target.info.context));
    }
  }

  // Test hook failure
  ov_error_set_autofill_hook(test_hook_failure);
  target = (struct ov_error_stack){0};
  target.info.type = ov_error_type_generic;
  target.info.code = ov_error_generic_fail;
  target.info.context = NULL;

  TEST_CHECK(!ov_error_autofill_message(&target, &err));
  TEST_CHECK(ov_error_is(&err, ov_error_type_generic, ov_error_generic_out_of_memory));
  OV_ERROR_DESTROY(&err);

  // Reset hook
  ov_error_set_autofill_hook(NULL);

  // Test that hook is properly reset
  target = (struct ov_error_stack){0};
  target.info.type = ov_error_type_generic;
  target.info.code = ov_error_generic_fail;
  target.info.context = NULL;

  TEST_CHECK(ov_error_autofill_message(&target, NULL));
  TEST_CHECK(target.info.context != NULL);
  TEST_CHECK(strcmp(target.info.context, "operation failed") == 0);

  if (target.info.context) {
    if (target.info.flag_context_is_static) {
      target.info.context = NULL;
      target.info.flag_context_is_static = 0;
    } else {
      OV_ARRAY_DESTROY(ov_deconster_(&target.info.context));
    }
  }
}

// Global variable to capture output hook calls for testing
static char *g_captured_output = NULL;

static void test_output_hook_capture(char const *str) {
  if (str) {
    size_t len = strlen(str);
    size_t existing_len = g_captured_output ? strlen(g_captured_output) : 0;
    size_t new_len = existing_len + len;

    if (!OV_ARRAY_GROW(&g_captured_output, new_len + 1, NULL)) {
      return; // Failed to allocate
    }
    strcpy(g_captured_output + existing_len, str);
  }
}

static void test_output_hook_suppress(char const *str) {
  (void)str; // Suppress output by not doing anything
}

static void test_ov_error_output_hook(void) {
  struct ov_error err = {0};

  // Reset captured output
  if (g_captured_output) {
    OV_ARRAY_DESTROY(&g_captured_output);
  }

  // Test setting output hook
  ov_error_set_output_hook(test_output_hook_capture);

  // Test error reporting with hook
  OV_ERROR_SET(&err, ov_error_type_generic, ov_error_generic_fail, "Test hook output");
  TEST_CHECK(OV_ERROR_REPORT(&err, "Hook test message"));
  verify_clean_state(&err);

  // Check that output was captured by hook
  if (TEST_CHECK(g_captured_output != NULL)) {
    TEST_CHECK(strstr(g_captured_output, "[ERROR] Hook test message") != NULL);
    TEST_CHECK(strstr(g_captured_output, "reported at error_test.c:") != NULL);
  }

  // Reset captured output
  if (g_captured_output) {
    OV_ARRAY_DESTROY(&g_captured_output);
  }

  // Test suppression hook
  ov_error_set_output_hook(test_output_hook_suppress);

  OV_ERROR_SET(&err, ov_error_type_generic, ov_error_generic_fail, "Test error");
  TEST_CHECK(OV_ERROR_REPORT(&err, "Suppressed message"));
  verify_clean_state(&err);

  // Output should not be captured (hook suppresses it)
  TEST_CHECK(g_captured_output == NULL);

  // Reset hook to default
  ov_error_set_output_hook(NULL);

  // Test that hook is properly reset (this would output to stderr by default)
  OV_ERROR_SET(&err, ov_error_type_generic, ov_error_generic_fail, "Test error");
  TEST_CHECK(OV_ERROR_REPORT(&err, "Default output test"));
  verify_clean_state(&err);

  // Clean up any remaining captured output
  if (g_captured_output) {
    OV_ARRAY_DESTROY(&g_captured_output);
  }
}

static void test_ov_error_reporting(void) {
  struct ov_error err = {0};

  // Test error reporting and cleanup
  OV_ERROR_SET(&err, ov_error_type_generic, ov_error_generic_fail, "Test error");
  TEST_CHECK(OV_ERROR_REPORT(&err, "Test operation failed"));
  verify_clean_state(&err);

  // Test no-op reporting when no error is set
  TEST_CHECK(OV_ERROR_REPORT(&err, "Should not report"));
  verify_clean_state(&err);
}

TEST_LIST = {
    {"ov_error_core_functionality", test_ov_error_core_functionality},
    {"ov_error_null_safety", test_ov_error_null_safety},
    {"ov_error_different_types", test_ov_error_different_types},
    {"ov_error_set_macro", test_ov_error_set_macro},
    {"ov_error_filepos_tracking", test_ov_error_filepos_tracking},
    {"ov_error_lifecycle_management", test_ov_error_lifecycle_management},
    {"ov_error_push_basic", test_ov_error_push_basic},
    {"ov_error_push_multiple_levels", test_ov_error_push_multiple_levels},
    {"ov_error_push_call_stack_overflow", test_ov_error_push_call_stack_overflow},
    {"ov_error_formatting", test_ov_error_formatting},
    {"ov_error_direct_parameters", test_ov_error_direct_parameters},
    {"ov_error_message_autofill", test_ov_error_message_autofill},
    {"ov_error_autofill_hook", test_ov_error_autofill_hook},
    {"ov_error_output_hook", test_ov_error_output_hook},
    {"ov_error_string_conversion", test_ov_error_string_conversion},
    {"ov_error_reporting", test_ov_error_reporting},
    {NULL, NULL},
};
