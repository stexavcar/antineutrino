#ifndef _HEAP_HEAP_H
#define _HEAP_HEAP_H

#include "heap/heap.h"
#include "heap/roots.h"
#include "values/values-inl.h"

namespace neutrino {

template <typename T>
Allocation<Buffer> Heap::new_buffer(uword size) {
  KLIDKIKS_ALLOC(AbstractBuffer, result, new_abstract_buffer(sizeof(T) * size, roots().buffer_layout()));
  return cast<Buffer>(result);
}

template <class C>
Allocation<C> Heap::new_singleton(Layout *type) {
  KLIDKIKS_ALLOC(Singleton, result, new_singleton(type));
  return cast<C>(result);
}

} // neutrino

#endif // _HEAP_HEAP_H
