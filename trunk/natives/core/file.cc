#include <stdio.h>
#include "runtime/builtins-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.h"

namespace neutrino {

/**
 * Support for file access.
 */

extern "C" Value *min(BuiltinArguments &args) {
  ASSERT_EQ(2, args.count());
  word first = cast<Smi>(to<Smi>(args[0]))->value();
  word second = cast<Smi>(to<Smi>(args[1]))->value();
  return (first < second) ? Smi::from_int(first) : Smi::from_int(second);
  
}

} // neutrino
