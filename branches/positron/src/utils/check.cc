#include "utils/check-inl.h"
#include "utils/string-inl.h"

namespace positron {

void Check::predicate_failed(SourceLocation here, string format,
    string details, const var_args &vars) {
  static const char *kHeader =
    "#\n"
    "# %:%: % failed";
  string_stream buf;
  buf.add(kHeader, args(string(here.file()), here.line(),
      format_bundle(format, vars)));
  buf.set_indent("#   ");
  buf.add("\n%", args(format_bundle(details, vars)));
  buf.set_indent(string());
  buf.add("\n#\n");
  Abort::abort("%", args(buf.raw_c_str()));

}

} // namespace positron
