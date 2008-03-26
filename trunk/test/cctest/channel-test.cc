#include "io/out-stream.h"
#include "runtime/runtime.h"
#include "values/channel.h"
#include "values/values-inl.h"
#include "utils/checks.h"

using namespace neutrino;

// This test is not for running, only compiling
void walk_dont_run() {
  IValue value = *((IValue*) 0);
  is<IString>(value);
  is<ITuple>(value);
  cast<IString>(value);
  cast<ITuple>(value);
}

void Test::test_integer() {
  ImageOutputStream stream;
  RawFValue *obj = stream.marshal(Smi::from_int(10));
  FrozenHeap heap(stream);
  FImmediate *image_value = heap.cook(obj);
  MethodDictionaryImpl dict;
  IValue value = dict.new_value(image_value);
  CHECK_EQ(vtInteger, value.type());
  CHECK(is<IInteger>(value));
  CHECK(!is<IString>(value));
  CHECK(!is<ITuple>(value));
  IInteger int_obj = cast<IInteger>(value);
  CHECK_EQ(10, int_obj.value());
}
