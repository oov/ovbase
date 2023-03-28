#include <stdio.h>
// xgettext --add-comments="trans:" -o test.pot test.c
int main(void) {
  // trans: gettext comment.
  printf(gettext("Hello world"));
  printf(ngettext("an apple", "apples"));
  printf(pgettext("Menu|File", "Open"));
  printf(pgettext("Menu|Printer", "Open"));
  return 0;
}
