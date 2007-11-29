#ifndef _HEAP_MEMORY
#define _HEAP_MEMORY

#include "utils/types.h"

namespace neutrino {

class Memory {
public:
  Memory();
  ~Memory();
  inline address allocate(uint32_t size);
  void collect_garbage();
private:
  friend class DisallowGarbageCollection;
  SemiSpace &young_space() { return *young_space_; }
  bool allow_garbage_collection() { return allow_garbage_collection_; }
  SemiSpace *young_space_;
  bool allow_garbage_collection_;
};

class DisallowGarbageCollection {
public:
  inline DisallowGarbageCollection();
  inline ~DisallowGarbageCollection();
private:
  Memory &memory() { return memory_; }
  Memory &memory_;
  bool previous_;
};

}

#endif // _HEAP_MEMORY
