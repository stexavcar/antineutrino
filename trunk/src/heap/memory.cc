#include "heap/memory-inl.h"

namespace neutrino {

Memory::Memory() {
  young_space_ = new SemiSpace(1024 * 1024);
  allow_garbage_collection_ = true;
}

Memory::~Memory() {
  delete young_space_;
}

void Memory::collect_garbage() {
  
}

} // neutrino
