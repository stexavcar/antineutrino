#ifndef _UTILS_LOG
#define _UTILS_LOG

#include "utils/string.h"
#include "utils/types.h"

namespace neutrino {


class SourceLocation {
public:
  SourceLocation(const char *file, int line) : file_(file), line_(line) { }
  string full_name() { return file_; }
  int line() { return line_; }
  string short_name();
private:
  const char *file_;
  int line_;
};


#define HERE() SourceLocation(__FILE__, __LINE__)


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
  static void set_verbosity(Verbosity v) { verbosity_ = v; }
  static Verbosity verbosity() { return verbosity_; }
private:
  static Verbosity verbosity_;
  SourceLocation &location() { return location_; }
  SourceLocation location_;
};

#define LOG() Log(HERE())

} // neutrino

#endif // _UTILS_LOG
