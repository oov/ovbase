#include "ovtest.h"

#include <inttypes.h>

#ifdef _WIN32
#  define STR_PH "%hs"
#else
#  define STR_PH "%s"
#endif

#ifdef USE_STR

static void test_str_cpy_free(void) {
  static char const *const test_str = "hello";
  struct str s = {0};
  if (TEST_SUCCEEDED_F(scpy(&s, test_str))) {
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, test_str) == 0);
    }
    TEST_CHECK(s.len == strlen(test_str));
    TEST_CHECK(s.cap > strlen(test_str));
  }

  static char const *const test_str2 = "good bye world";
  if (TEST_SUCCEEDED_F(scpy(&s, test_str2))) {
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, test_str2) == 0);
    }
    TEST_CHECK(s.len == strlen(test_str2));
    TEST_CHECK(s.cap > strlen(test_str2));
  }

  static char const *const test_str3 = "";
  if (TEST_SUCCEEDED_F(scpy(&s, test_str3))) {
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, test_str3) == 0);
    }
    TEST_CHECK(s.len == strlen(test_str3));
    TEST_CHECK(s.cap > strlen(test_str3));
  }

  if (TEST_SUCCEEDED_F(sfree(&s))) {
    TEST_CHECK(s.ptr == NULL);
    TEST_CHECK(s.len == 0);
    TEST_CHECK(s.cap == 0);
  }

  TEST_EISG_F(scpy(&s, NULL), err_invalid_arugment);
}

static void test_str_cpym(void) {
  static char const *const s1 = "hello";
  static char const *const s2 = "world";
  static char const *const expected = "helloworld";
  struct str s = {0};
  if (TEST_SUCCEEDED_F(scpym(&s, s1, s2))) {
    TEST_CHECK(strcmp(s.ptr, expected) == 0);
    TEST_MSG("expected " STR_PH, expected);
    TEST_MSG("got " STR_PH, s.ptr);
  }
  if (TEST_SUCCEEDED_F(scpym(&s, s1, s2))) {
    TEST_CHECK(strcmp(s.ptr, expected) == 0);
    TEST_MSG("expected " STR_PH, expected);
    TEST_MSG("got " STR_PH, s.ptr);
  }
  TEST_SUCCEEDED_F(sfree(&s));
}

static void test_str_ncpy(void) {
  static char const *const test_str = "hello";
  struct str s = {0};
  if (TEST_SUCCEEDED_F(sncpy(&s, test_str, 2))) {
    TEST_CHECK(s.ptr != NULL);
    TEST_CHECK(s.len == 2);
    TEST_CHECK(s.cap > 2);
    TEST_CHECK(s.ptr[s.len] == L'\0');
    TEST_SUCCEEDED_F(sfree(&s));
  }
  if (TEST_SUCCEEDED_F(sncpy(&s, test_str, 5))) {
    if (TEST_SUCCEEDED_F(sncpy(&s, test_str, 2))) {
      TEST_CHECK(s.ptr != NULL);
      TEST_CHECK(s.len == 2);
      TEST_CHECK(s.cap > 2);
      TEST_CHECK(s.ptr[s.len] == L'\0');
      TEST_SUCCEEDED_F(sfree(&s));
    }
  }
  if (TEST_SUCCEEDED_F(sncpy(&s, test_str, 100))) {
    TEST_CHECK(s.ptr != NULL);
    TEST_CHECK(s.len == 5);
    TEST_CHECK(s.cap > 5);
    TEST_CHECK(s.ptr[s.len] == L'\0');
    TEST_SUCCEEDED_F(sfree(&s));
  }
}

static void test_str_cat_free(void) {
  static char const *const test_str = "hello";
  struct str s = {0};
  TEST_SUCCEEDED_F(scpy(&s, test_str));
  static char const *const test_str2 = "world";
  if (TEST_SUCCEEDED_F(scat(&s, test_str2))) {
    static char const *const expected_str = "helloworld";
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, expected_str) == 0);
    }
    TEST_CHECK(s.len == strlen(expected_str));
    TEST_CHECK(s.cap > strlen(expected_str));
  }

  static char const *const test_str3 = "";
  if (TEST_SUCCEEDED_F(scat(&s, test_str3))) {
    static char const *const expected_str = "helloworld";
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, expected_str) == 0);
    }
    TEST_CHECK(s.len == strlen(expected_str));
    TEST_CHECK(s.cap > strlen(expected_str));
  }

  if (TEST_SUCCEEDED_F(sfree(&s))) {
    TEST_CHECK(s.ptr == NULL);
    TEST_CHECK(s.len == 0);
    TEST_CHECK(s.cap == 0);
  }

  if (TEST_SUCCEEDED_F(scat(&s, test_str))) {
    if (TEST_CHECK(s.ptr != NULL)) {
      TEST_CHECK(strcmp(s.ptr, test_str) == 0);
    }
    TEST_CHECK(s.len == strlen(test_str));
    TEST_CHECK(s.cap > strlen(test_str));
  }
  TEST_SUCCEEDED_F(sfree(&s));
  TEST_EISG_F(scat(&s, NULL), err_invalid_arugment);
}

static void test_str_catm(void) {
  static char const *const s1 = "hello";
  static char const *const s2 = "world";
  static char const *const expected1 = "helloworld";
  static char const *const expected2 = "helloworldhelloworld";
  struct str s = {0};
  if (TEST_SUCCEEDED_F(scatm(&s, s1, s2))) {
    TEST_CHECK(strcmp(s.ptr, expected1) == 0);
    TEST_MSG("expected " STR_PH, expected1);
    TEST_MSG("got " STR_PH, s.ptr);
  }
  if (TEST_SUCCEEDED_F(scatm(&s, s1, s2))) {
    TEST_CHECK(strcmp(s.ptr, expected2) == 0);
    TEST_MSG("expected " STR_PH, expected2);
    TEST_MSG("got " STR_PH, s.ptr);
  }
  TEST_SUCCEEDED_F(sfree(&s));
}

static void test_str_ncat(void) {
  static char const *const test_str = "hello";
  static char const *const test2_str = "world";
  struct str s = {0};
  if (TEST_SUCCEEDED_F(sncat(&s, test_str, 2))) {
    TEST_CHECK(s.ptr != NULL);
    TEST_CHECK(s.len == 2);
    TEST_CHECK(s.cap > 2);
    TEST_CHECK(strcmp(s.ptr, "he") == 0);
    TEST_SUCCEEDED_F(sfree(&s));
  }
  if (TEST_SUCCEEDED_F(sncat(&s, test_str, 100))) {
    TEST_CHECK(s.ptr != NULL);
    TEST_CHECK(s.len == 5);
    TEST_CHECK(s.cap > 5);
    TEST_CHECK(s.ptr[s.len] == L'\0');
    TEST_CHECK(strcmp(s.ptr, "hello") == 0);
    TEST_SUCCEEDED_F(sfree(&s));
  }

  if (TEST_SUCCEEDED_F(sncat(&s, test_str, 2))) {
    if (TEST_SUCCEEDED_F(sncat(&s, test2_str, 2))) {
      TEST_CHECK(s.ptr != NULL);
      TEST_CHECK(s.len == 4);
      TEST_CHECK(s.cap > 4);
      TEST_CHECK(strcmp(s.ptr, "hewo") == 0);
    }
    TEST_SUCCEEDED_F(sfree(&s));
  }
  if (TEST_SUCCEEDED_F(sncat(&s, test_str, 2))) {
    if (TEST_SUCCEEDED_F(sncat(&s, test2_str, 100))) {
      TEST_CHECK(s.ptr != NULL);
      TEST_CHECK(s.len == 7);
      TEST_CHECK(s.cap > 7);
      TEST_CHECK(strcmp(s.ptr, "heworld") == 0);
    }
    TEST_SUCCEEDED_F(sfree(&s));
  }
}

static void test_str_grow(void) {
  struct str s = {0};
  if (TEST_SUCCEEDED_F(sgrow(&s, 1))) {
    TEST_CHECK(s.ptr != NULL);
    TEST_CHECK(s.len == 0);
    TEST_CHECK(s.cap >= 1);
  }
  TEST_SUCCEEDED_F(sfree(&s));
}

static void test_str_str(void) {
  struct str s = str_unmanaged("hello");
  ptrdiff_t pos = 0, expected = -1;
  if (TEST_SUCCEEDED_F(sstr(&s, "!", &pos))) {
    TEST_CHECK(pos == expected);
    TEST_MSG("expected %td", expected);
    TEST_MSG("got %td", pos);
  }
  expected = 1;
  if (TEST_SUCCEEDED_F(sstr(&s, "e", &pos))) {
    TEST_CHECK(pos == expected);
    TEST_MSG("expected %td", expected);
    TEST_MSG("got %td", pos);
  }
  expected = 4;
  if (TEST_SUCCEEDED_F(sstr(&s, "o", &pos))) {
    TEST_CHECK(pos == expected);
    TEST_MSG("expected %td", expected);
    TEST_MSG("got %td", pos);
  }
  TEST_EISG_F(sstr(&s, NULL, NULL), err_invalid_arugment);
  TEST_EISG_F(sstr(&s, "e", NULL), err_null_pointer);
}

static void test_str_replace_all(void) {
  struct str s = {0};
  if (TEST_SUCCEEDED_F(scpy(&s, "hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&s, "o", "xx"))) {
      static char const *const expected = "hellxx wxxrld";
      TEST_CHECK(strcmp(s.ptr, expected) == 0);
      TEST_MSG("expected " STR_PH, expected);
      TEST_MSG("got " STR_PH, s.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&s, "hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&s, "o", ""))) {
      static char const *const expected = "hell wrld";
      TEST_CHECK(strcmp(s.ptr, expected) == 0);
      TEST_MSG("expected " STR_PH, expected);
      TEST_MSG("got " STR_PH, s.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&s, "hello world"))) {
    if (TEST_SUCCEEDED_F(sreplace_all(&s, "d", "xx"))) {
      static char const *const expected = "hello worlxx";
      TEST_CHECK(strcmp(s.ptr, expected) == 0);
      TEST_MSG("expected " STR_PH, expected);
      TEST_MSG("got " STR_PH, s.ptr);
    }
  }
  if (TEST_SUCCEEDED_F(scpy(&s, "hello world"))) {
    TEST_SUCCEEDED_F(sreplace_all(&s, "", "xx"));
    TEST_SUCCEEDED_F(sreplace_all(&s, "a", "xx"));
    TEST_EISG_F(sreplace_all(&s, NULL, NULL), err_invalid_arugment);
    TEST_EISG_F(sreplace_all(&s, "o", NULL), err_invalid_arugment);
    TEST_EISG_F(sreplace_all(&s, NULL, "xx"), err_invalid_arugment);
  }
  TEST_SUCCEEDED_F(sfree(&s));
}

static void test_str_const(void) {
  static char const *const s = "hello";
  ptrdiff_t pos = 0;
  if (TEST_SUCCEEDED_F(sstr(&str_unmanaged_const(s), "o", &pos))) {
    TEST_CHECK(pos == 4);
  }
}
#endif // USE_STR

TEST_LIST = {
#ifdef USE_STR
    {"test_str_cpy_free", test_str_cpy_free},
    {"test_str_cpym", test_str_cpym},
    {"test_str_ncpy", test_str_ncpy},
    {"test_str_cat_free", test_str_cat_free},
    {"test_str_catm", test_str_catm},
    {"test_str_ncat", test_str_ncat},
    {"test_str_grow", test_str_grow},
    {"test_str_str", test_str_str},
    {"test_str_replace_all", test_str_replace_all},
    {"test_str_const", test_str_const},
#endif // USE_STR
    {NULL, NULL},
};
