#include "plankton/plankton-inl.h"
#include "runtime/runtime-inl.h"
#include "value/condition-inl.h"
#include "test-inl.h"

using namespace neutrino;

boole test_allocation() {
  Runtime runtime;
  try runtime.initialize();
  try alloc {runtime.heap()} InstanceDescriptor *raw_desc(runtime.roots().descriptor_descriptor(), 19);
  assert raw_desc->instance_type() == Value::tInstance;
  assert raw_desc->field_count() == 19;
  ref_block<1> protect(runtime.refs());
  ref<InstanceDescriptor> desc = protect(raw_desc);
  assert desc->instance_type() == Value::tInstance;
  assert desc->field_count() == 19;
  runtime.heap().collect_garbage();
  assert *desc != raw_desc;
  assert desc->instance_type() == Value::tInstance;
  assert desc->field_count() == 19;
  return Success::make();
}

TEST(heap) {
  assert test_allocation().has_succeeded();
}
