#include "utils/check-inl.h"
#include "utils/string-inl.h"

namespace neutrino {

void Check::predicate_failed(SourceLocation here, string format,
    string details, const var_args &vars) {
  static const char *kHeader =
    "#\n"
    "# %:%: % failed";
  string_stream buf;
  buf.add(kHeader, args(string(here.file()), here.line(),
      format_bundle(format, vars)));
  if (!details.is_empty()) {
    buf.set_indent("#   ");
    buf.add("\n%", args(format_bundle(details, vars)));
    buf.set_indent(string());
  }
  buf.add("\n#\n");
  Abort::abort("%", args(buf.raw_c_str()));
}

void Check::check_failed(SourceLocation here, string source) {
  static const char *kMessage =
    "#\n"
    "# %:%: % failed\n"
    "#\n";
  string_stream buf;
  buf.add(kMessage, args(string(here.file()), here.line(), source));
  Abort::abort("%", args(buf.raw_c_str()));
}

} // namespace neutrino
