#include "compiler/compile-utils-inl.h"
#include "heap/ref-inl.h"
#include "heap/values-inl.h"

namespace neutrino {

void heap_list::initialize() {
  ref<Tuple> tuple = factory().new_tuple(16);
  data_ = new_permanent(*tuple);
}

heap_list::~heap_list() {
  data().dispose();
}

void heap_list::append(ref<Value> value) {
  ASSERT(length() < data().length() - 1);
  data().set(length(), value);
  length_++;
}

}
