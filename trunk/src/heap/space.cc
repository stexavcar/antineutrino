#include "heap/space-inl.h"
#include "utils/checks.h"

namespace neutrino {

SemiSpace::SemiSpace(uint32_t capacity) {
  cursor_ = 0;
  capacity_ = capacity;
  data_ = new int8_t[capacity];
}

bool SemiSpace::contains(address addr) {
  return (start() <= addr) && (addr < end());
}

SemiSpace::~SemiSpace() {
  delete[] data_;
}

} // neutrino
