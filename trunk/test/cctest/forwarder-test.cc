#include "cctest/tests-inl.h"
#include "heap/heap-inl.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::simple_forwarder() {
  LocalRuntime runtime;
  Tuple *empty = runtime.roots().empty_tuple();
  Forwarder *forwarder = cast<Forwarder>(runtime.heap().new_transparent_forwarder(empty));
  CHECK(!is<Smi>(forwarder));
  CHECK(!is<Object>(forwarder));
  CHECK(!is<Signal>(forwarder));
  CHECK_IS(Forwarder, forwarder);
  CHECK_IS(Value, forwarder);
  CHECK(forwarder->descriptor()->target() == empty);
  Data *target_val = to<Tuple>(forwarder);
  CHECK_IS(Tuple, target_val);
  CHECK(target_val == empty);
  Data *string_val = to<String>(forwarder);
  CHECK(is<Nothing>(string_val));
}

void Test::smi_forwarder() {
  LocalRuntime runtime;
  Value *obj = Smi::from_int(17);
  Forwarder *forwarder = cast<Forwarder>(runtime.heap().new_transparent_forwarder(obj));
  CHECK(!is<Smi>(forwarder));
  CHECK(!is<Object>(forwarder));
  CHECK(!is<Signal>(forwarder));
  CHECK_IS(Forwarder, forwarder);
  CHECK_IS(Value, forwarder);
  CHECK(to<Smi>(forwarder) == obj);
  CHECK(to<Immediate>(forwarder) == obj);
  CHECK_IS(Nothing, to<String>(forwarder));
}
