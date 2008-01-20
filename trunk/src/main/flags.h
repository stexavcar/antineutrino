#ifndef _MAIN_FLAGS
#define _MAIN_FLAGS

#include "utils/flags.h"
#include "utils/globals.h"

namespace neutrino {

class Flags {
public:
  DECLARE_FLAG(bool, print_stats_on_exit, false);
};

} // neutrino

#endif // _MAIN_FLAGS
