#ifndef _CCTEST_NUNIT
#define _CCTEST_NUNIT

namespace neutrino {

typedef void (test_callback_t)();

class NUnitTest {
public:
  NUnitTest(string file, string name, test_callback_t *callback);
  static int main(list<const char *> args);
  string file() { return file_; }
  string name() { return name_; }
  NUnitTest *prev() { return prev_; }
  void run() { callback_(); }
private:
  static void print_test_list();
  static uword run_matching_tests(string file_or_name);
  static uword run_matching_tests(string file, string name);
  static uword run_all_tests();

  static NUnitTest *first_;
  string file_;
  string name_;
  test_callback_t *callback_;
  NUnitTest *prev_;
};

} // neutrino

#endif // _CCTEST_NUNIT
