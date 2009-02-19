#include <cstdio>
#include <cstdlib>

#include "platform/abort.h"
#include "utils/string-inl.h"

namespace neutrino {

void Abort::abort(string format, const var_args &vars) {
  string_stream buf;
  buf.add(format, vars);
  fprintf(stderr, "%s", buf.raw_string().start());
  ::abort();
}

void Abort::install_signal_handlers() {
  // no default implementation
}

} // namespace neutrino
