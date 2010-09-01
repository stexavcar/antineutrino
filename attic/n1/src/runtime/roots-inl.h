#ifndef _RUNTIME_ROOTS_INL
#define _RUNTIME_ROOTS_INL

#include "runtime/roots.h"

namespace neutrino {

RootIterator::RootIterator(Roots &roots)
    : roots_(roots), index_(0) { }

bool RootIterator::has_next() {
  return index_ < Roots::kCount;
}

Value **RootIterator::next() {
  return roots_.get(index_++);
}

} // namespace neutrino

#endif // _RUNTIME_ROOTS_INL
