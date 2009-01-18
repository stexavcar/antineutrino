#ifndef _PLATFORM_ABORT
#define _PLATFORM_ABORT

#include "utils/global.h"
#include "utils/string.h"

namespace positron {

class Abort {
public:
  static void abort(string format, const var_args &args);
  static void install_signal_handlers();
};

} // namespace positron

#endif // _PLATFORM_ABORT
