#include <ctime>

#include "platform/abort.h"
#include "utils/checks.h"
#include "values/values-inl.pp.h"

namespace neutrino {

static Conditions kDefaultConditions;
Conditions *Conditions::current_ = NULL;


Conditions &Conditions::get() {
  if (current_ == NULL) return kDefaultConditions;
  else return *current_;
}


void Conditions::notify(Condition cause) {
  // ignore
}


MAKE_ENUM_INFO_HEADER(Condition)
#define MAKE_ENTRY(Name) MAKE_ENUM_INFO_ENTRY(cn##Name)
eConditions(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()


void Conditions::check_failed(string file_name, int line_number,
    string source, bool value, Condition cause) {
  notify(cause);
  static string kErrorMessage =
    "#\n"
    "# %:%: CHECK(%) failed\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, elms(file_name, line_number, source));
  abort(buf.raw_string());
}


void Conditions::check_predicate_failed(string file_name, int line_number,
    const variant &expected, string expected_source, const variant &value,
    string value_source, string name, Condition cause) {
  notify(cause);
  static string kErrorMessage =
    "#\n"
    "# %:%: % % % failed\n"
    "#   expected: %\n"
    "#   found: %\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, elms(file_name, line_number, expected_source,
      name, value_source, expected, value));
  abort(buf.raw_string());
}


void Conditions::unreachable(string file_name, int line_number,
    Condition cause) {
  notify(cause);
  static string kErrorMessage =
    "#\n"
    "# %:%: Unreachable code\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, elms(file_name, line_number));
  abort(buf.raw_string());
}


void Conditions::unhandled(string file_name, int line_number,
    string enum_name, word value, AbstractEnumInfo &info,
    Condition cause) {
  notify(cause);
  string name = info.get_name_for(value);
  static string kErrorMessage =
    "#\n"
    "# %:%: Unhandled % value %\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, elms(file_name, line_number, enum_name, name));
  abort(buf.raw_string());
}


void Conditions::error_occurred(string format, const var_args &args) {
  string_buffer buf;
  buf.printf(format, args);
  abort(buf.raw_string());
}


void Conditions::abort(string message) {
  Abort::abort(message);
}


} // namespace neutrino
