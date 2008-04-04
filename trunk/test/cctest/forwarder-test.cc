#include "cctest/tests-inl.h"
#include "heap/heap-inl.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::simple_forwarder() {
  LocalRuntime runtime;
  Tuple *empty = runtime.roots().empty_tuple();
  Forwarder *forwarder = cast<Forwarder>(runtime.heap().new_forwarder(Forwarder::fwOpen, empty));
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
  Forwarder *forwarder = cast<Forwarder>(runtime.heap().new_forwarder(Forwarder::fwOpen, obj));
  CHECK(!is<Smi>(forwarder));
  CHECK(!is<Object>(forwarder));
  CHECK(!is<Signal>(forwarder));
  CHECK_IS(Forwarder, forwarder);
  CHECK_IS(Value, forwarder);
  CHECK(to<Smi>(forwarder) == obj);
  CHECK(to<Immediate>(forwarder) == obj);
  CHECK_IS(Nothing, to<String>(forwarder));
}

void Test::check_closed_immutable() {
#ifdef DEBUG
  LocalRuntime runtime;
  Value *obj = Smi::from_int(17);
  Forwarder *forwarder = cast<Forwarder>(runtime.heap().new_forwarder(Forwarder::fwOpen, obj));
  forwarder->descriptor()->set_type(Forwarder::fwClosed);
  CHECK_ABORTS(cnForwarderState, forwarder->descriptor()->set_target(Smi::from_int(19)));
#endif // DEBUG
}

void Test::check_stay_closed() {
#ifdef DEBUG
  LocalRuntime runtime;
  Value *obj = Smi::from_int(17);
  Forwarder *forwarder = cast<Forwarder>(runtime.heap().new_forwarder(Forwarder::fwOpen, obj));
  forwarder->descriptor()->set_type(Forwarder::fwClosed);
  CHECK_ABORTS(cnForwarderState, forwarder->descriptor()->set_type(Forwarder::fwOpen));
#endif // DEBUG
}
