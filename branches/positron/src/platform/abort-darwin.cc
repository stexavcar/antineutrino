#include <cstdio>
#include <cstdlib>

#include "platform/abort.h"
#include "utils/string-inl.h"

namespace positron {

void Abort::abort(string format, const var_args &vars) {
  string_stream buf;
  buf.add(format, vars);
  fprintf(stderr, "%s", buf.raw_c_str().start());
  ::abort();
}

} // namespace positron
