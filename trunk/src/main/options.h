#ifndef _MAIN_OPTIONS
#define _MAIN_OPTIONS

#include "utils/flags.h"
#include "utils/globals.h"

namespace neutrino {

class Options {
public:
  static bool print_stats_on_exit;
  static list<string> images;
};

} // neutrino

#endif // _MAIN_OPTIONS
