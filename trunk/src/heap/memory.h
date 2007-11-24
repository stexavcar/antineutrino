#ifndef _HEAP_MEMORY
#define _HEAP_MEMORY

#include "heap/space.h"

namespace neutrino {

class Memory {
public:
  Memory();
  ~Memory();
  inline address allocate(uint32_t size);
private:
  SemiSpace &young_space() { return *young_space_; }
  SemiSpace *young_space_;
};

}

#endif // _HEAP_MEMORY
