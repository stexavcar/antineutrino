#ifndef _ROOTS_INL
#define _ROOTS_INL

#include "heap/roots.h"
#include "utils/types-inl.h"

namespace neutrino {

RootIterator::RootIterator(Roots &roots)
    : roots_(roots), index_(0) { }

bool RootIterator::has_next() {
  return index_ < Roots::kCount;
}

Value **RootIterator::next() {
  return &roots_.get(index_++);
}

Value *&Roots::get(uint32_t n) {
  ASSERT(n < kCount);
  return entries_[n];
}

}

#endif // _ROOTS_INL
