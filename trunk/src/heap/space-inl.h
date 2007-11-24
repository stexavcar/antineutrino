#ifndef _HEAP_SPACE_INL
#define _HEAP_SPACE_INL

#include "heap/pointer-inl.h"
#include "heap/space.h"

namespace neutrino {

address SemiSpace::allocate(uint32_t size) {
  ValuePointer::is_aligned(size);
  if (cursor_ + size > capacity_) return 0;
  address result = start() + cursor_;
  cursor_ += size;
  return result;
}

}

#endif // _HEAP_SPACE_INL
