#ifndef _HEAP_SPACE_INL
#define _HEAP_SPACE_INL

#include "heap/pointer-inl.pp.h"
#include "heap/space.h"

namespace neutrino {

address SemiSpace::allocate(uword size) {
  ValuePointer::is_aligned(size);
  if (cursor_ + size > capacity_) return 0;
  address result = start() + cursor_;
  cursor_ += size;
  return result;
}

SemiSpaceIterator::SemiSpaceIterator(SemiSpace &space)
    : space_(space), offset_(0) {
}

bool SemiSpaceIterator::has_next() {
  return offset_ < space_.cursor_;
}

Object *SemiSpaceIterator::next() {
  Object* result = ValuePointer::tag_as_object(space_.data_ + offset_);
  offset_ += result->size_in_memory();
  return result;
}

}

#endif // _HEAP_SPACE_INL
