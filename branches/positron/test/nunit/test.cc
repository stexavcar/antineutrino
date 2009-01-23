#include <cstdio>

#include "test-inl.h"
#include "utils/vector-inl.h"
#include "utils/smart-ptrs-inl.h"

namespace neutrino {

UnitTest *UnitTest::first_ = NULL;

UnitTest::UnitTest(string file, string name, test_callback_t *callback)
  : name_(name)
  , callback_(callback) {
  // Find the base name of this test.
  const char *found_basename = strrchr(file.start(), '/');
  char *basename = strdup(found_basename ? found_basename + 1 : file.start());
  // Drop the extension, if there is one.
  char *extension = strrchr(basename, '.');
  if (extension) *extension = 0;
  // Install this test in the list of tests
  file_ = basename;
  prev_ = first_;
  first_ = this;
}

void UnitTest::print_test_list() {
  UnitTest *current = first_;
  while (current != NULL) {
    printf("%s/%s\n", current->file_.start(),
        current->name_.start());
    current = current->prev_;
  }
}

word UnitTest::run_matching_tests(string file, string name) {
  UnitTest *current = first_;
  word run_count = 0;
  while (current != NULL) {
    if (current->file() == file && current->name() == name) {
      current->run();
      run_count++;
    }
    current = current->prev();
  }
  return run_count;
}

word UnitTest::run_matching_tests(string file_or_name) {
  UnitTest *current = first_;
  word run_count = 0;
  while (current != NULL) {
    if (current->file() == file_or_name || current->name() == file_or_name) {
      current->run();
      run_count++;
    }
    current = current->prev();
  }
  return run_count;
}

word UnitTest::run_all_tests() {
  UnitTest *current = first_;
  word run_count = 0;
  while (current != NULL) {
    current->run();
    run_count++;
    current = current->prev();
  }
  return run_count;
}

vector<const char*> UnitTest::args_;
bool UnitTest::spawned_ = false;

int UnitTest::main(vector<const char*> args) {
  args_ = args;
  Abort::install_signal_handlers();
  word total_run_count = 0;
  bool print_run_count = true;
  for (word i = 1; i < args.length(); i++) {
    string arg = args[i];
    if (arg == "--list") {
      print_test_list();
      print_run_count = false;
    } else if (arg == "--all") {
      run_all_tests();
    } else if (arg == "--spawned") {
      spawned_ = true;
    } else {
      word current_run_count = 0;
      own_vector<const char> str_copy(string::dup(arg).chars());
      char *testname = strchr(const_cast<char*>(str_copy.start()), '/');
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
        printf("No tests matched '%s'.\n", arg.start());
      }
      total_run_count += current_run_count;
    }
  }
  if (print_run_count == 0 && print_run_count)
    printf("No tests were run.\n");
  return 0;
}

word PseudoRandom::next() {
  a_ = (1351 * a_) + (2963 * b_) + 9373;
  b_ = (5197 * b_) + (9399 * a_) + 5137;
  return (9113 * a_) + (7437 * b_) + 6927;
}

} // neutrino

int main(int argc, const char *argv[]) {
  neutrino::vector<const char*> args(argv, argc);
  return neutrino::UnitTest::main(args);
}
