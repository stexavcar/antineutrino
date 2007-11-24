#ifndef _HEAP_MEMORY_INL
#define _HEAP_MEMORY_INL

#include "heap/memory.h"
#include "heap/space-inl.h"
#include "utils/types.h"

namespace neutrino {

address Memory::allocate(uint32_t size) {
  return young_space().allocate(size);
}

}

#endif // _HEAP_MEMORY_INL
