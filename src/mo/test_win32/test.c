#include <stdio.h>
// Regenerate test.pot:
//   xgettext --add-comments="trans:" --from-code=UTF-8 --package-name=TestSoftware --package-version=v0.0.0
//   --add-location=file --copyright-holder="TestSoftware Developers" --no-wrap --sort-output --output test.pot test.c
// Create intial *.po:
//   msginit --no-translator --input=test.pot --no-wrap --locale=ja.UTF-8
//   msginit --no-translator --input=test.pot --no-wrap --locale=en.UTF-8
//   msginit --no-translator --input=test.pot --no-wrap --locale=zh_CN.UTF-8
// Apply changes in *.pot to *.po:
//   msgmerge --no-wrap --sort-output --output ja.po ja.po test.pot
//   msgmerge --no-wrap --sort-output --output en.po en.po test.pot
//   msgmerge --no-wrap --sort-output --output zh_CN.po zh_CN.po test.pot
// Regenerate *.mo:
//   msgfmt --output-file="res/1041_RCData_MO.bin" --no-hash ja.po
//   msgfmt --output-file="res/1033_RCData_MO.bin" --no-hash en.po
//   msgfmt --output-file="res/2052_RCData_MO.bin" --no-hash zh_CN.po
//   touch test.rc
int main(void) {
  // trans: This dagger helps UTF-8 detection. You don't need to translate this.
  gettext_noop("†");
  printf(gettext("Hello world"));
  printf(ngettext("an apple", "%d apples", 2), 2);
  printf(pgettext("Menu|File", "Open"));
  printf(pgettext("Menu|Printer", "Open"));
  return 0;
}
