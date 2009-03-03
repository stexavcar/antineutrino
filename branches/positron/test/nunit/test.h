#ifndef _PUNIT_TEST
#define _PUNIT_TEST

#include "check.h"
#include "utils/string.h"
#include "utils/vector.h"

namespace neutrino {

typedef void (test_callback_t)();

class UnitTest {
public:
  UnitTest(string file, string name, test_callback_t *callback);
  static int main(vector<const char *> args);
  string file() { return file_; }
  string name() { return name_; }
  UnitTest *prev() { return prev_; }
  void run() { callback_(); }
  static vector<const char*> &args() { return args_; }
  static bool spawned() { return spawned_; }
private:
  static void print_test_list();
  static word run_matching_tests(string file_or_name);
  static word run_matching_tests(string file, string name);
  static word run_all_tests();

  static vector<const char*> args_;
  static bool spawned_;
  static UnitTest *first_;
  string file_;
  string name_;
  test_callback_t *callback_;
  UnitTest *prev_;
};

class PseudoRandom {
public:
  PseudoRandom(word seed, word other = 257) : a_(seed), b_(other) { }
  void reset(word seed, word other = 257) { a_ = seed; b_ = other; }
  word next();
  TaggedInteger *next_tagged_integer();
private:
  word a_;
  word b_;
};

} // neutrino

#endif // _PUNIT_TEST
