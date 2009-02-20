#include "runtime/heap-inl.h"
#include "runtime/roots-inl.h"
#include "value/value-inl.h"

namespace neutrino {

Roots::Roots() {
  RootIterator iter(*this);
  while (iter.has_next())
    *iter.next() = 0;
}

boole Roots::initialize(Heap &heap) {
  try alloc DescriptorDescriptor *descriptor_descriptor() in heap;
  descriptor_descriptor->set_descriptor(descriptor_descriptor);
  set_descriptor_descriptor(descriptor_descriptor);
#define ALLOCATE_ROOT(n, Type, name, args)                           \
  try alloc Type *name##_value(args) in heap;                        \
  set_##name(name##_value);
eSimpleRoots(ALLOCATE_ROOT)
#undef ALLOCATE_ROOT
  return Success::make();
}

Value **Roots::get(word n) {
  return &entries_[n];
}

} // namespace neutrino
