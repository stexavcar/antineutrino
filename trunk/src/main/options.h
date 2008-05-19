#ifndef _MAIN_OPTIONS
#define _MAIN_OPTIONS

#include "utils/flags.h"
#include "utils/globals.h"

namespace neutrino {

class Options {
public:
  static bool print_stats_on_exit;
  static bool trace_signals;
  static bool COND_FLAG(IS_PARANOID, check_stack_height, false);
  static bool COND_FLAG(IS_DEBUG, trace_interpreter, false);
  static list<string> images;
  static list<string> libs;
  static list<string> args;
};

} // neutrino

#endif // _MAIN_OPTIONS
