#include "compiler/compile-utils-inl.h"
#include "heap/gc-safe.h"
#include "heap/ref-inl.h"
#include "runtime/runtime-inl.h"
#include "values/values-inl.h"

namespace neutrino {

Signal *heap_list::initialize() {
  stack_ref_block<> safe(runtime().refs());
  @protect ref<Tuple> tuple = runtime().factory().new_tuple(16);
  data_ = runtime().refs().new_persistent(*tuple);
  return Success::make();
}

heap_list::~heap_list() {
  data().dispose();
}

Signal *heap_list::extend_capacity() {
  stack_ref_block<> safe(runtime().refs());
  uword capacity = data().length();
  uword new_capacity = grow_value(capacity);
  @protect ref<Tuple> new_data = runtime().factory().new_tuple(new_capacity);
  for (uword i = 0; i < capacity; i++)
    new_data->set(i, data()->get(i));
  data().dispose();
  set_data(runtime().refs().new_persistent(*new_data));
  return Success::make();
}

Signal *heap_list::append(ref<Value> value) {
  if (length() >= data().length())
    @try extend_capacity();
  data().set(length(), value);
  length_++;
  return Success::make();
}

} // neutrino
