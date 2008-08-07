#include "compiler/compile-utils-inl.h"
#include "heap/gc-safe.h"
#include "heap/ref-inl.h"
#include "runtime/runtime-inl.h"
#include "values/values-inl.h"

namespace neutrino {

likely heap_list::initialize() {
  ref_block<> protect(runtime().refs());
  @check(probably) ref<Tuple> tuple = runtime().factory().new_tuple(16);
  data_ = runtime().refs().new_persistent(*tuple);
  return Success::make();
}

heap_list::~heap_list() {
  data().dispose();
}

likely heap_list::extend_capacity() {
  ref_block<> protect(runtime().refs());
  uword capacity = data()->length();
  uword new_capacity = grow_value(capacity);
  @check(probably) ref<Tuple> new_data = runtime().factory().new_tuple(new_capacity);
  for (uword i = 0; i < capacity; i++)
    new_data->set(i, data()->get(i));
  data().dispose();
  set_data(runtime().refs().new_persistent(*new_data));
  return Success::make();
}

likely heap_list::append(ref<Value> value) {
  if (length() >= data()->length())
    @try(likely) extend_capacity();
  data()->set(length(), *value);
  length_++;
  return Success::make();
}

} // neutrino
