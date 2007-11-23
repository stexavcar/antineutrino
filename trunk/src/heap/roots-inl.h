#ifndef _ROOTS_INL
#define _ROOTS_INL

#include "heap/roots.h"
#include "utils/types-inl.h"

namespace neutrino {

template <typename D> void Roots::for_each(void (*callback)(Value**, D),
    D data) {
#define VISIT_FIELD(n, Type, name, Name, NAME, allocator)            \
  callback(pointer_cast<Value*>(&entries_[n]), data);
FOR_EACH_ROOT(VISIT_FIELD)
#undef VISIT_FIELD
}

Object *&Roots::get(uint32_t n) {
  ASSERT(n < kCount);
  return entries_[n];
}

}

#endif // _ROOTS_INL
