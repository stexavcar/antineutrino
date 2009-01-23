#ifndef _PLATFORM_ABORT
#define _PLATFORM_ABORT

#include "utils/global.h"
#include "utils/string.h"

namespace neutrino {

class Abort {
public:
  static void abort(string format, const var_args &args);

  // Installs neutrino-specific signal handlers.
  static void install_signal_handlers();

  // Removes neutrino's signal handlers.
  static void uninstall_signal_handlers();

  // Disposes all open resources.  Should only be used when terminating
  // abruptly since resources will otherwise clean themselves up.
  static void release_resources();

  static void register_resource(abstract_resource &that);
  static void unregister_resource(abstract_resource &that);
  static word resource_count();
private:
  static bool has_cleaned_up_;
  static abstract_resource *first_;
  static abstract_resource *last_;
};

} // namespace neutrino

#endif // _PLATFORM_ABORT
