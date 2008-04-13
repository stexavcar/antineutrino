#include <stdarg.h>

#include "platform/abort.h"
#include "utils/checks.h"
#include "values/values-inl.h"

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

void Conditions::check_is_failed(string file_name, int line_number,
    string type_name, uword type_tag, Data *data,
    string value_source, Condition cause) {
  notify(cause);
#ifdef DEBUG
  static string kErrorMessage =
    "#\n"
    "# %:%: CHECK_IS(%, %) failed\n"
    "#   expected: %\n"
    "#   found: %\n"
    "#\n";
  EnumInfo<InstanceType> enum_info;
  string expected_name = Layout::layout_name(type_tag);
  uword value_tag = Layout::tag_of(data);
  string value_type_name = Layout::layout_name(value_tag);
  string_buffer buf;
  buf.printf(kErrorMessage, file_name, line_number, type_name,
      value_source, expected_name, value_type_name);
#else // DEBUG
  static string kErrorMessage =
    "#\n"
    "# %:%: CHECK_IS(%, %) failed\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, file_name, line_number, type_name,
      value_source);
#endif // DEBUG  
  abort(buf.raw_string());
}

void Conditions::check_failed(string file_name, int line_number,
    string source, bool value, Condition cause) {
  notify(cause);
  static string kErrorMessage =
    "#\n"
    "# %:%: CHECK(%) failed\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, file_name, line_number, source);
  abort(buf.raw_string());
}

void Conditions::check_eq_failed(string file_name, int line_number,
    int expected, string expected_source, int value, string value_source,
    Condition cause) {
  notify(cause);
  static string kErrorMessage =
    "#\n"
    "# %:%: CHECK_EQ(%, %) failed\n"
    "#   expected: %\n"
    "#   found: %\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, file_name, line_number, expected_source,
      value_source, expected, value);
  abort(buf.raw_string());
}

void Conditions::check_eq_failed(string file_name, int line_number,
    string expected, string expected_source, string value,
    string value_source, Condition cause) {
  notify(cause);
  static string kErrorMessage =
    "#\n"
    "# %:%: CHECK_EQ(%, %) failed\n"
    "#   expected: %\n"
    "#   found: %\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, file_name, line_number, expected_source,
      value_source, expected, value);
  abort(buf.raw_string());
}

void Conditions::check_ge_failed(string file_name, int line_number,
    word value, string value_source, word limit, string limit_source,
    Condition cause) {
  notify(cause);
  static string kErrorMessage =
    "#\n"
    "# %:%: CHECK_GE(%, %) failed\n"
    "#   value: %\n"
    "#   limit: %\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, file_name, line_number, value_source,
      limit_source, value, limit);
  abort(buf.raw_string());
}

void Conditions::check_lt_failed(string file_name, int line_number,
    int value, string value_source, int limit, string limit_source,
    Condition cause) {
  notify(cause);
  static string kErrorMessage =
    "#\n"
    "# %:%: CHECK_LT(%, %) failed\n"
    "#   value: %\n"
    "#   limit: %\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, file_name, line_number, value_source,
      limit_source, value, limit);
  abort(buf.raw_string());
}

void Conditions::check_eq_failed(string file_name, int line_number,
    Value *expected, string expected_source, Value *value,
    string value_source, Condition cause) {
  notify(cause);
  static string kErrorMessage =
    "#\n"
    "# %:%: CHECK_EQ(%, %) failed\n"
    "#   expected: %{x}\n"
    "#   found: %{x}\n"
    "#\n";
  string_buffer buf;
  buf.printf(kErrorMessage, file_name, line_number, expected_source,
      value_source, reinterpret_cast<word>(expected),
      reinterpret_cast<word>(value));
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
  buf.printf(kErrorMessage, file_name, line_number);
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
  buf.printf(kErrorMessage, file_name, line_number, enum_name, name);
  abort(buf.raw_string());
}

void Conditions::error_occurred(string format, list<fmt_elm> elms) {
  string_buffer buf;
  buf.printf(format, elms);
  abort(buf.raw_string());
}

void Conditions::error_occurred(string format) {
  error_occurred(format, list<fmt_elm>());
}

void Conditions::error_occurred(string format, fmt_elm elm1) {
  const uword argc = 1;
  fmt_elm argv[argc] = { elm1 };
  error_occurred(format, list<fmt_elm>(argv, argc));
}

void Conditions::error_occurred(string format, fmt_elm elm1,
    fmt_elm elm2) {
  const uword argc = 2;
  fmt_elm argv[argc] = { elm1, elm2 };
  error_occurred(format, list<fmt_elm>(argv, argc));
}

void Conditions::error_occurred(string format, fmt_elm elm1,
    fmt_elm elm2, fmt_elm elm3) {
  const uword argc = 3;
  fmt_elm argv[argc] = { elm1, elm2, elm3 };
  error_occurred(format, list<fmt_elm>(argv, argc));
}

void Conditions::abort(string message) {
  Abort::abort(message);
}

} // namespace neutrino
