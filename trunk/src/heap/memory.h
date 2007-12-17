#ifndef _HEAP_MEMORY
#define _HEAP_MEMORY

#include "utils/globals.h"
#include "utils/types.h"

namespace neutrino {

class Memory {
public:
  Memory(Heap &heap);
  ~Memory();
  inline address allocate(uint32_t size);
  void collect_garbage();
  SemiSpace &young_space() { return *young_space_; }
  IF_MONITOR(static int32_t bytes_allocated_);
  IF_MONITOR(static int32_t objects_allocated_);
  IF_MONITOR(static int32_t live_objects_);
  IF_MONITOR(static int32_t live_bytes_);
private:
  void notify_monitors();
  friend class DisallowGarbageCollection;
  friend class GarbageCollectionMonitor;
  Heap &heap() { return heap_; }
  bool allow_garbage_collection() { return allow_garbage_collection_; }
  GarbageCollectionMonitor *monitor_chain() { return monitor_chain_; }
  Heap &heap_;
  SemiSpace *young_space_;
  bool allow_garbage_collection_;
  GarbageCollectionMonitor *monitor_chain_;
};

/**
 * Stack-allocating an instance of this class causes garbage
 * collection to abort.  This is useful if you know that garbage
 * collection must not occur since it will alert you if it does.
 */
class DisallowGarbageCollection {
public:
  inline DisallowGarbageCollection(Memory &memory);
  inline ~DisallowGarbageCollection();
private:
  Memory &memory() { return memory_; }
  bool previous() { return previous_; }
  Memory &memory_;
  bool previous_;
};

/**
 * A stack-allocated monitor will be notified when a garbage
 * collection occurs.  This is useful if a function needs to know
 * whether or not a garbage collection has occurred during its
 * execution.
 */
class GarbageCollectionMonitor {
public:
  inline GarbageCollectionMonitor(Memory &memory);
  inline ~GarbageCollectionMonitor();
  bool has_collected_garbage() { return has_collected_garbage_; }
private:
  friend class Memory;
  Memory &memory() { return memory_; }
  GarbageCollectionMonitor *previous() { return previous_; }
  void notify() { has_collected_garbage_ = true; }
  GarbageCollectionMonitor *previous_;
  Memory &memory_;
  bool has_collected_garbage_;
};

} // neutrino

#endif // _HEAP_MEMORY
