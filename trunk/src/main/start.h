#ifndef _MAIN_MAIN
#define _MAIN_MAIN

#include "utils/globals.h"
#include "utils/list-inl.h"

namespace neutrino {

class Library {
public:
  static likely initialize_runtime(list<char*> &args, Runtime &runtime);
  static likely run(Runtime &runtime);
  static int main(list<char*> &args);
  static likely start(list<char*> &args);
  static int report_result(likely result);

private:
  static likely initialize(list<char*> &args);
  static void on_option_error(string message);
  static Image *read_image(string name);
  static likely build_arguments(Runtime &runtime);
  static DynamicLibraryCollection *load_dynamic_libraries();
  static bool has_been_initialized_;
};

} // neutrino

#endif // _MAIN_MAIN
