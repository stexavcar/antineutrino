#include <ctime>


#include "utils/log.h"
#include "utils/string-inl.h"


namespace neutrino {

class LogOptions : public OptionCollection {
public:
  LogOptions() {
    REGISTER_FLAG(log_level, Log::WARN);
  }
  Log::Verbosity log_level() { return static_cast<Log::Verbosity>(log_level_); }
private:
  word log_level_;
};

static LogOptions &log_options() {
  static LogOptions instance;
  return instance;
}

OptionCollection &Log::options() {
  return log_options();
}

const char *SourceLocation::short_name() const {
  const char *loc = strrchr(file_, '/');
  if (loc) return loc + 1;
  else return file_;
}


// Note that because of the call to localtime this function is not
// thread protect.  Who came up with the idea of sharing the result
// value between all invocations of localtime?!?
static string format_current_time(vector<char> &buf) {
  time_t raw_now = ::time(NULL);
  struct tm *now = ::localtime(&raw_now);
  size_t count = ::strftime(buf.start(), buf.length(), "%X", now);
  buf[count] = '\0'; // just to be on the safe side
  return string(buf.start(), count);
}


void Log::info(string format, const var_args &elms) {
  if (log_options().log_level() < INFO) return;
  string_stream buf;
  embed_vector<char, 16> time_buf;
  buf.add("[%] I %:%: %",
      vargs(format_current_time(time_buf), string(location().short_name()),
          location().line(), format_bundle(format, elms)));
  buf.raw_string().println();
}


void Log::warn(string format, const var_args &elms) {
  if (log_options().log_level() < WARN) return;
  string_stream buf;
  embed_vector<char, 16> time_buf;
  buf.add("[%] W %:%: %",
      vargs(format_current_time(time_buf), string(location().short_name()),
          location().line(), format_bundle(format, elms)));
  buf.raw_string().println();
}


void Log::error(string format, const var_args &elms) {
  if (log_options().log_level() < ERROR) return;
  string_stream buf;
  embed_vector<char, 16> time_buf;
  buf.add("[%] E %:%: %",
      vargs(format_current_time(time_buf), string(location().short_name()),
          location().line(), format_bundle(format, elms)));
  buf.raw_string().println();
}


} // namespace neutrino
