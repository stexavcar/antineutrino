#ifndef _PLATFORM_ABORT
#define _PLATFORM_ABORT

namespace neutrino {

class Abort {
public:
  static bool setup_signal_handler();
  static void abort();
};

}

#endif // _PLATFORM_ABORT
