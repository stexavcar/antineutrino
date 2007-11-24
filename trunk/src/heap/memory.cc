#include "heap/memory-inl.h"

namespace neutrino {

Memory::Memory() {
  young_space_ = new SemiSpace(1024 * 1024);
}

Memory::~Memory() {
  delete young_space_;
}

} // neutrino
