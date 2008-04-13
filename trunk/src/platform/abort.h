#ifndef _PLATFORM_ABORT
#define _PLATFORM_ABORT

#include "utils/string.h"

namespace neutrino {

class Resource {
public:
  void install();
  void uninstall();
  virtual void cleanup() = 0;
private:
  friend class Abort;
  Resource *prev_;
  Resource *next_;
};

class Abort {
public:
  static bool setup_signal_handler();
  static void abort(string message);
  static void cleanup_resources();
private:
  friend class Resource;
  static Resource *first_;
  static Resource *last_;
};

}

#endif // _PLATFORM_ABORT
