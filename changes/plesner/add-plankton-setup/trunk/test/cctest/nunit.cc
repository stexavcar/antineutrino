#include "cctest/nunit-inl.h"
#include "utils/array-inl.h"
#include "utils/list-inl.h"
#include "utils/smart-ptrs-inl.h"

namespace neutrino {

NUnitTest *NUnitTest::first_ = NULL;

NUnitTest::NUnitTest(string file, string name, test_callback_t *callback)
  : name_(name)
  , callback_(callback) {
  // Find the base name of this test.
  char *basename = strrchr(file.chars(), '/');
  if (!basename) basename = strdup(file.chars());
  else basename = strdup(basename + 1);
  // Drop the extension, if there is one.
  char *extension = strrchr(basename, '.');
  if (extension) *extension = 0;
  // Install this test in the list of tests
  file_ = basename;
  prev_ = first_;
  first_ = this;
}

void NUnitTest::print_test_list() {
  NUnitTest *current = first_;
  while (current != NULL) {
    printf("%s:%s\n", current->file_.chars(), current->name_.chars());
    current = current->prev_;
  }
}

uword NUnitTest::run_matching_tests(string file, string name) {
  NUnitTest *current = first_;
  uword run_count = 0;
  while (current != NULL) {
    if (current->file() == file && current->name() == name) {
      current->run();
      run_count++;
    }
    current = current->prev();
  }
  return run_count;
}

uword NUnitTest::run_matching_tests(string file_or_name) {
  NUnitTest *current = first_;
  uword run_count = 0;
  while (current != NULL) {
    if (current->file() == file_or_name || current->name() == file_or_name) {
      current->run();
      run_count++;
    }
    current = current->prev();
  }
  return run_count;
}

uword NUnitTest::run_all_tests() {
  NUnitTest *current = first_;
  uword run_count = 0;
  while (current != NULL) {
    current->run();
    run_count++;
    current = current->prev();
  }
  return run_count;
}

int NUnitTest::main(list<const char*> args) {
  uword total_run_count = 0;
  bool print_run_count = true;
  for (uword i = 1; i < args.length(); i++) {
    string arg = args[i];
    if (arg == "--list") {
      print_test_list();
      print_run_count = false;
    } else if (arg == "--all") {
      run_all_tests();
    } else {
      uword current_run_count = 0;
      own_array<char> str_copy(NEW_ARRAY(strdup(arg.chars()), arg.length()));
      char *testname = strchr(str_copy.start(), ':');
      if (testname) {
        // Split the string in two by nulling the colon
        *testname = 0;
        // There is a colon in the test name; find the exact test
        current_run_count = run_matching_tests(str_copy.start(), testname + 1);
      } else {
        // There is no colon; run any tests whose file or test name
        // match
        current_run_count = run_matching_tests(arg);
      }
      if (current_run_count == 0) {
        printf("No tests matched '%s'.\n", arg.chars());
      }
      total_run_count += current_run_count;
    }
  }
  if (print_run_count)
    printf("Ran %i tests.\n", total_run_count);
  return 0;
}

} // neutrino

int main(int argc, const char *argv[]) {
  neutrino::list<const char*> args(argv, argc);
  return neutrino::NUnitTest::main(args);
}
