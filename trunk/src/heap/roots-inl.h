#ifndef _ROOTS_INL
#define _ROOTS_INL

#include "heap/roots.h"
#include "utils/types-inl.h"

namespace neutrino {

template <typename D> void Roots::for_each(void (*callback)(Value**, D),
    D data) {
#define VISIT_FIELD(n, Type, name, NAME, allocator)                  \
  callback(pointer_cast<Value*>(&name##_), data);
FOR_EACH_ROOT(VISIT_FIELD)
#undef VISIT_FIELD
}

}

#endif // _ROOTS_INL
