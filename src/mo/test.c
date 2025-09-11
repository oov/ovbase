#include "mo.c"
#include <ovtest.h>

#include <ovarray.h>

#include <stdio.h>
#ifndef _WIN32
#  include <locale.h>
#endif

static struct mo *open_mo(uint8_t *mobuf) {
  FILE *f = fopen(OVBASE_SOURCE_DIR "/mo/test/en_US.mo", "rb");
  if (!TEST_CHECK(f != NULL)) {
    return NULL;
  }
  size_t mosize = fread(mobuf, 1, 4096, f);
  fclose(f);
  if (!TEST_CHECK(mosize > 0)) {
    return NULL;
  }
  struct mo *mo = NULL;
  struct ov_error err = {0};
  if (!TEST_CHECK(mo_parse(&mo, mobuf, mosize, &err))) {
    OV_ERROR_DESTROY(&err);
    return NULL;
  }
  return mo;
}

static void test_mo(void) {
  uint8_t mobuf[4096];
  struct mo *mp = open_mo(mobuf);
  if (!mp) {
    return;
  }
  struct mo_msg *msg = mp->msg;
  TEST_CHECK(msg->id != NULL && msg->id_len == 0); // header
  msg = mp->msg + 1;
  TEST_CHECK(msg->id != NULL && memcmp(msg->id, "Hello world", msg->id_len) == 0);
  TEST_CHECK(msg->str != NULL && memcmp(msg->str, "Hello world2", msg->str_len) == 0);
  msg = mp->msg + 2;
  TEST_CHECK(msg->id != NULL && memcmp(msg->id, "Menu|File\x04Open", msg->id_len) == 0);
  TEST_CHECK(msg->str != NULL && memcmp(msg->str, "Open file", msg->str_len) == 0);
  msg = mp->msg + 3;
  TEST_CHECK(msg->id != NULL && memcmp(msg->id, "Menu|Printer\x04Open", msg->id_len) == 0);
  TEST_CHECK(msg->str != NULL && memcmp(msg->str, "Open printer", msg->str_len) == 0);
  msg = mp->msg + 4;
  TEST_CHECK(msg->id != NULL && memcmp(msg->id, "an apple\0apples", msg->id_len) == 0);
  TEST_CHECK(msg->str != NULL && memcmp(msg->str, "an apple2\0apples2", msg->str_len) == 0);

  msg = find(mp, "Hello world");
  TEST_CHECK(msg == mp->msg + 1);
  msg = find(mp, "Menu|Printer\x04Open");
  TEST_CHECK(msg == mp->msg + 3);
  msg = find(mp, "an apple\0apples");
  TEST_CHECK(msg == mp->msg + 4);

  char const *got = NULL, *expected = NULL;
  TEST_CHECK(strcmp(expected = "Hello world2", got = mo_gettext(mp, "Hello world")) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "Hello_world", got = mo_gettext(mp, "Hello_world")) == 0);
  TEST_MSG("expected %s got %s", expected, got);

  TEST_CHECK(strcmp(expected = "Open file", got = mo_pgettext(mp, "Menu|File", "Open")) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "Open printer", got = mo_pgettext(mp, "Menu|Printer", "Open")) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "Open", got = mo_pgettext(mp, "Menu|XXX", "Open")) == 0);
  TEST_MSG("expected %s got %s", expected, got);

  TEST_CHECK(strcmp(expected = "apples2", got = mo_ngettext(mp, "an apple", "apples", 0)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "an apple2", got = mo_ngettext(mp, "an apple", "apples", 1)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples2", got = mo_ngettext(mp, "an apple", "apples", 2)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples2", got = mo_ngettext(mp, "an apple", "apples3", 0)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "an apple2", got = mo_ngettext(mp, "an apple", "apples3", 1)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples2", got = mo_ngettext(mp, "an apple", "apples3", 2)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples", got = mo_ngettext(mp, "an apple3", "apples", 0)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "an apple3", got = mo_ngettext(mp, "an apple3", "apples", 1)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples", got = mo_ngettext(mp, "an apple3", "apples", 2)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples3", got = mo_ngettext(mp, "an apple3", "apples3", 0)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "an apple3", got = mo_ngettext(mp, "an apple3", "apples3", 1)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  TEST_CHECK(strcmp(expected = "apples3", got = mo_ngettext(mp, "an apple3", "apples3", 2)) == 0);
  TEST_MSG("expected %s got %s", expected, got);
  mo_free(&mp);
}

static void test_mo_get_preferred_ui_languages(void) {
#ifdef _WIN32
  wchar_t *str = NULL;
#else
  char *str = NULL;
#endif

#ifndef _WIN32
  char const *const old_locale = setlocale(LC_MESSAGES, NULL);
  if (!TEST_CHECK(setlocale(LC_ALL, "") != NULL)) {
    goto cleanup;
  }
#endif
  {
    struct ov_error err = {0};
    if (!TEST_CHECK(mo_get_preferred_ui_languages(&str, &err))) {
      OV_ERROR_DESTROY(&err);
      goto cleanup;
    }
  }
#ifndef _WIN32
  if (!TEST_CHECK(setlocale(LC_ALL, old_locale) != NULL)) {
    goto cleanup;
  }
#endif
  {
    size_t n = 0;
    size_t pos = 0;
    while (str[pos] != NSTR('\0')) {
#ifdef _WIN32
#  define LEN wcslen
#else
#  define LEN strlen
#endif
      pos += LEN(str + pos) + 1;
#undef LEN
      ++n;
    }
    TEST_CHECK(n > 0);
  }
cleanup:
  if (str) {
    OV_ARRAY_DESTROY(&str);
  }
}

#ifdef _WIN32
static void test_mo_win32_locale(void) {
  struct mo *mp = NULL;
  {
    struct ov_error err = {0};
    bool result = mo_parse_from_resource_ex(&mp, NULL, L"ko-KR\0en_US\0", &err);
    if (!TEST_CHECK(!result && ov_error_is(&err, ov_error_type_generic, ov_error_generic_not_found))) {
      OV_ERROR_DESTROY(&err);
      goto cleanup;
    }
    OV_ERROR_DESTROY(&err);
  }
  {
    struct ov_error err = {0};
    if (!TEST_CHECK(mo_parse_from_resource_ex(&mp, NULL, L"zh-CN\0", &err))) {
      OV_ERROR_DESTROY(&err);
      goto cleanup;
    }
  }
  TEST_CHECK(strcmp(mo_gettext(mp, "Hello world"), "世界你好") == 0);
  mo_free(&mp);
  {
    struct ov_error err = {0};
    if (!TEST_CHECK(mo_parse_from_resource_ex(&mp, NULL, L"zh-TW\0", &err))) {
      OV_ERROR_DESTROY(&err);
      goto cleanup;
    }
  }
  TEST_CHECK(strcmp(mo_gettext(mp, "Hello world"), "世界你好") == 0);
cleanup:
  mo_free(&mp);
}

static void test_mo_win32(void) {
  struct mo *mp = NULL;
  {
    struct ov_error err = {0};
    if (!TEST_CHECK(mo_parse_from_resource_ex(&mp, NULL, L"ja-JP\0", &err))) {
      OV_ERROR_DESTROY(&err);
      return;
    }
  }

  TEST_CHECK(strcmp(mo_gettext(mp, "Hello world"), "ハローワールド") == 0);
  TEST_CHECK(strcmp(mo_gettext(mp, "Hello_world"), "Hello_world") == 0);

  TEST_CHECK(strcmp(mo_pgettext(mp, "Menu|File", "Open"), "ファイルを開く") == 0);
  TEST_CHECK(strcmp(mo_pgettext(mp, "Menu|XXX", "Open"), "Open") == 0);

  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple", "%d apples", 0), "%d個のりんご") == 0);
  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple", "%d apples", 1), "%d個のりんご") == 0);
  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple", "%d apples", 2), "%d個のりんご") == 0);

  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple", "%d apples3", 0), "%d個のりんご") == 0);
  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple", "%d apples3", 1), "%d個のりんご") == 0);
  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple", "%d apples3", 2), "%d個のりんご") == 0);

  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple3", "%d apples", 0), "%d apples") == 0);
  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple3", "%d apples", 1), "an apple3") == 0);
  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple3", "%d apples", 2), "%d apples") == 0);

  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple3", "%d apples3", 0), "%d apples3") == 0);
  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple3", "%d apples3", 1), "an apple3") == 0);
  TEST_CHECK(strcmp(mo_ngettext(mp, "an apple3", "%d apples3", 2), "%d apples3") == 0);

  mo_free(&mp);
}
#endif

TEST_LIST = {
    {"test_mo", test_mo},
    {"test_mo_get_preferred_ui_languages", test_mo_get_preferred_ui_languages},
#ifdef _WIN32
    {"test_mo_win32_locale", test_mo_win32_locale},
    {"test_mo_win32", test_mo_win32},
#endif
    {NULL, NULL},
};
