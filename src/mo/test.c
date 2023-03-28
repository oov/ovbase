#include "mo.c"
#include "ovtest.h"

#include <stdio.h>

static struct mo *open_mo(uint8_t *mobuf) {
  FILE *f = fopen(SOURCE_DIR "/mo/test/en_US.mo", "rb");
  if (!TEST_CHECK(f != NULL)) {
    return NULL;
  }
  size_t mosize = fread(mobuf, 1, 4096, f);
  fclose(f);
  if (!TEST_CHECK(mosize > 0)) {
    return NULL;
  }
  struct mo *mo = NULL;
  if (!TEST_SUCCEEDED_F(mo_parse(&mo, mobuf, mosize))) {
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

#ifdef _WIN32
static void test_mo_win32(void) {
  struct mo *mp = NULL;
  struct wstr ws = {0};
  if (!TEST_SUCCEEDED_F(mo_parse_from_resource(&mp, NULL, MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT)))) {
    return;
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
  ereport(sfree(&ws));
}
#endif

TEST_LIST = {
    {"test_mo", test_mo},
#ifdef _WIN32
    {"test_mo_win32", test_mo_win32},
#endif
    {NULL, NULL},
};
