#ifndef _HEAP_MEMORY_INL
#define _HEAP_MEMORY_INL

#include "heap/memory.h"
#include "heap/space-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/types.h"

namespace neutrino {

address Memory::allocate(uword size) {
  return young_space().allocate(size);
}

DisallowGarbageCollection::DisallowGarbageCollection(Memory &memory)
    : memory_(memory) {
  previous_ = memory.allow_garbage_collection();
  memory.allow_garbage_collection_ = false;
}

DisallowGarbageCollection::~DisallowGarbageCollection() {
  memory().allow_garbage_collection_ = previous();
}

GarbageCollectionMonitor::GarbageCollectionMonitor(Memory &memory)
  : previous_(memory.monitor_chain())
  , memory_(memory)
  , has_collected_garbage_(false) {
  memory.monitor_chain_ = this;
}

GarbageCollectionMonitor::~GarbageCollectionMonitor() {
  memory().monitor_chain_ = previous_;
}

}

#endif // _HEAP_MEMORY_INL