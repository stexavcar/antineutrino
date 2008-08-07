#include "cctest/nunit-inl.h"
#include "heap/heap-inl.h"
#include "values/values-inl.h"

using namespace neutrino;

TEST(simple_forwarder) {
  LocalRuntime runtime;
  Tuple *empty = runtime.roots().empty_tuple();
  Forwarder *forwarder = runtime.heap().new_forwarder(Forwarder::fwOpen, empty).value();
  CHECK(!is<Smi>(forwarder));
  CHECK(!is<Object>(forwarder));
  CHECK(!is<Signal>(forwarder));
  CHECK_IS(Forwarder, forwarder);
  CHECK_IS(Value, forwarder);
  CHECK(forwarder->descriptor()->target() == empty);
  maybe<Tuple> target_val = to<Tuple>(forwarder);
  CHECK_IS(Tuple, target_val.value());
  CHECK(target_val.value() == empty);
  maybe<String> string_val = to<String>(forwarder);
  CHECK_IS(TypeMismatch, string_val.signal());
}

TEST(smi_forwarder) {
  LocalRuntime runtime;
  Value *obj = Smi::from_int(17);
  Forwarder *forwarder = runtime.heap().new_forwarder(Forwarder::fwOpen, obj).value();
  CHECK(!is<Smi>(forwarder));
  CHECK(!is<Object>(forwarder));
  CHECK(!is<Signal>(forwarder));
  CHECK_IS(Forwarder, forwarder);
  CHECK_IS(Value, forwarder);
  CHECK(to<Smi>(forwarder).value() == obj);
  CHECK(to<Immediate>(forwarder).value() == obj);
  CHECK_IS(TypeMismatch, to<String>(forwarder).signal());
}

TEST(check_closed_immutable) {
#ifdef DEBUG
  LocalRuntime runtime;
  Value *obj = Smi::from_int(17);
  Forwarder *forwarder = runtime.heap().new_forwarder(Forwarder::fwOpen, obj).value();
  forwarder->descriptor()->set_type(Forwarder::fwClosed);
  CHECK_ABORTS(cnForwarderState, forwarder->descriptor()->set_target(Smi::from_int(19)));
#endif // DEBUG
}

TEST(check_stay_closed) {
#ifdef DEBUG
  LocalRuntime runtime;
  Value *obj = Smi::from_int(17);
  Forwarder *forwarder = runtime.heap().new_forwarder(Forwarder::fwOpen, obj).value();
  forwarder->descriptor()->set_type(Forwarder::fwClosed);
  CHECK_ABORTS(cnForwarderState, forwarder->descriptor()->set_type(Forwarder::fwOpen));
#endif // DEBUG
}
