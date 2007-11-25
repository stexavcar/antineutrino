#ifndef _HEAP_MEMORY_INL
#define _HEAP_MEMORY_INL

#include "heap/memory.h"
#include "heap/space-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/types.h"

namespace neutrino {

address Memory::allocate(uint32_t size) {
  return young_space().allocate(size);
}

DisallowGarbageCollection::DisallowGarbageCollection()
    : memory_(Runtime::current().heap().memory()) {
  previous_ = memory().allow_garbage_collection();
  memory().allow_garbage_collection_ = false;
}

DisallowGarbageCollection::~DisallowGarbageCollection() {
  memory().allow_garbage_collection_ = previous_;
}

}

#endif // _HEAP_MEMORY_INL
