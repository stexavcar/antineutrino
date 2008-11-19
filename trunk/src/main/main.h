#ifndef _MAIN_MAIN
#define _MAIN_MAIN

#include "utils/globals.h"
#include "utils/list-inl.h"

namespace neutrino {

class Main {
public:
  static int main(list<char*> &args);
  static likely run_system(list<char*> &args);
  static void on_option_error(string message);
  static Image *read_image(string name);
  static likely build_arguments(Runtime &runtime);
  static DynamicLibraryCollection *load_dynamic_libraries();
};

} // neutrino

#endif // _MAIN_MAIN
