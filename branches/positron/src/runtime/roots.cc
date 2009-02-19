#include "runtime/heap-inl.h"
#include "runtime/roots.h"
#include "value/value-inl.h"

namespace neutrino {

Roots::Roots() {
  RootIterator iter(*this);
  while (iter.has_next())
    *iter.next() = 0;
}

boole Roots::initialize(Heap &heap) {
  array<uint8_t> memory = heap.allocate(sizeof(DescriptorDescriptor));
  if (memory.is_empty()) return InternalError::make(InternalError::ieHeapExhaustion);
  DescriptorDescriptor *descriptor_descriptor = new (memory) DescriptorDescriptor();
  descriptor_descriptor->set_descriptor(descriptor_descriptor);
  set_descriptor_descriptor(descriptor_descriptor);
  return Success::make();
}

RootIterator::RootIterator(Roots &roots)
    : roots_(roots), index_(0) { }

bool RootIterator::has_next() {
  return index_ < Roots::kCount;
}

Value **RootIterator::next() {
  return roots_.get(index_++);
}

Value **Roots::get(word n) {
  return &entries_[n];
}

} // namespace neutrino
