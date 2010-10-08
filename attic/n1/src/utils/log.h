#ifndef _UTILS_LOG
#define _UTILS_LOG

#include "utils/flags.h"
#include "utils/global.h"
#include "utils/string.h"
#include "utils/types.h"

namespace neutrino {

class Log {
public:
  enum Verbosity {
    QUIET = 0,
    ERROR = QUIET + 1,
    WARN = ERROR + 1,
    INFO = WARN + 1,
    ALL = INFO + 1
  };
  explicit Log(SourceLocation location) : location_(location) { }
  void info(string format, const var_args &args);
  void warn(string format, const var_args &args);
  void error(string format, const var_args &args);
  static OptionCollection &options();
private:
  SourceLocation &location() { return location_; }
  SourceLocation location_;
};

#define LOG() Log(dHere)

} // neutrino

#endif // _UTILS_LOG