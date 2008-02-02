#include "heap/space-inl.h"
#include "utils/checks.h"

namespace neutrino {

SemiSpace::SemiSpace(uword capacity) {
  cursor_ = 0;
  capacity_ = capacity;
  data_ = new int8_t[capacity];
}

bool SemiSpace::contains(Object *obj) {
  address addr = ValuePointer::address_of(obj);
  return (start() <= addr) && (addr < end());
}

SemiSpace::~SemiSpace() {
  delete[] data_;
}

} // neutrino
