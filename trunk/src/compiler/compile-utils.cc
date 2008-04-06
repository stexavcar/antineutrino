#include "compiler/compile-utils-inl.h"
#include "heap/factory.h"
#include "heap/ref-inl.h"
#include "runtime/runtime-inl.h"
#include "values/values-inl.h"

namespace neutrino {

void heap_list::initialize() {
  ref<Tuple> tuple = runtime().factory().new_tuple(16);
  data_ = new_persistent(*tuple);
}

heap_list::~heap_list() {
  data().dispose();
}

void heap_list::extend_capacity() {
  ref_scope scope(runtime().refs());
  uword capacity = data().length();
  uword new_capacity = grow_value(capacity);
  ref<Tuple> new_data = runtime().factory().new_tuple(new_capacity);
  for (uword i = 0; i < capacity; i++)
    new_data.set(i, data().get(runtime().refs(), i));
  data().dispose();
  set_data(new_persistent(*new_data));
}

void heap_list::append(ref<Value> value) {
  if (length() >= data().length())
    extend_capacity();
  data().set(length(), value);
  length_++;
}

}
