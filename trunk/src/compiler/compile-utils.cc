#include "compiler/compile-utils-inl.h"
#include "heap/factory.h"
#include "heap/ref-inl.h"
#include "heap/values-inl.h"

namespace neutrino {

void heap_list::initialize() {
  ref<Tuple> tuple = factory().new_tuple(16);
  data_ = new_persistent(*tuple);
}

heap_list::~heap_list() {
  data().dispose();
}

void heap_list::extend_capacity() {
  RefScope scope;
  uint32_t capacity = data().length();
  uint32_t new_capacity = grow_value(capacity);
  ref<Tuple> new_data = factory().new_tuple(new_capacity);
  for (uint32_t i = 0; i < capacity; i++)
    new_data.set(i, data().get(i));
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
