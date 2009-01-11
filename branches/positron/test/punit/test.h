#ifndef _PUNIT_TEST
#define _PUNIT_TEST

#include "check.h"
#include "utils/string.h"
#include "utils/array.h"

namespace positron {

typedef void (test_callback_t)();

class UnitTest {
public:
  UnitTest(string file, string name, test_callback_t *callback);
  static int main(array<const char *> args);
  string file() { return file_; }
  string name() { return name_; }
  UnitTest *prev() { return prev_; }
  void run() { callback_(); }
private:
  static void print_test_list();
  static word run_matching_tests(string file_or_name);
  static word run_matching_tests(string file, string name);
  static word run_all_tests();

  static UnitTest *first_;
  string file_;
  string name_;
  test_callback_t *callback_;
  UnitTest *prev_;
};

} // positron

#endif // _PUNIT_TEST
