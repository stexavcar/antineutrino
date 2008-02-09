#include <stdio.h>
#include "runtime/builtins-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.h"

namespace neutrino {

/*
 * Used by the natives test
 */

extern "C" Data *min(BuiltinArguments &args) {
  ASSERT_EQ(2, args.count());
  word first = cast<Smi>(to<Smi>(args[0]))->value();
  word second = cast<Smi>(to<Smi>(args[1]))->value();
  return (first < second) ? Smi::from_int(first) : Smi::from_int(second);
}

/*
 * Used by the forwarder test.
 */

extern "C" Data *make_forwarder(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  return args.runtime().heap().new_transparent_forwarder(args.self());
}

extern "C" Data *set_target(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  Forwarder *forwarder = cast<Forwarder>(args.self());
  forwarder->descriptor()->set_target(args[0]);
  return args.runtime().roots().vhoid();
}

} // neutrino
