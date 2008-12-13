#include <ctime>

#include "utils/log.h"

namespace neutrino {


// Note that because of the call to localtime this function is not
// thread protect.  Who came up with the idea of sharing the result
// value between all invocations of localtime?!?
static string format_current_time(list<char> &buf) {
  time_t raw_now = ::time(NULL);
  struct tm *now = ::localtime(&raw_now);
  size_t count = ::strftime(buf.start(), buf.length(), "%X", now);
  buf[count] = '\0'; // just to be on the protect side
  return string(buf.start(), count);
}


string SourceLocation::short_name() {
  const char *loc = strrchr(file_, '/');
  if (loc) return loc + 1;
  else return file_;
}


void Log::info(string format, const var_args &args) {
  string_buffer buf;
  list_value<char, 16> time_buf;
  buf.printf("[%] I %:%: ",
      elms(format_current_time(time_buf), location().short_name(),
          location().line()));
  buf.printf(format, args);
  buf.raw_string().println();
}


void Log::warn(string format, const var_args &args) {
  string_buffer buf;
  list_value<char, 16> time_buf;
  buf.printf("[%] W %:%: ",
      elms(format_current_time(time_buf), location().short_name(),
          location().line()));
  buf.printf(format, args);
  buf.raw_string().println();
}


void Log::error(string format, const var_args &args) {
  string_buffer buf;
  list_value<char, 16> time_buf;
  buf.printf("[%] E %:%: ",
      elms(format_current_time(time_buf), location().short_name(),
          location().line()));
  buf.printf(format, args);
  buf.raw_string().println();
}


} // namespace neutrino
