#ifndef _HEAP_HEAP_H
#define _HEAP_HEAP_H

#include "heap/heap.h"
#include "heap/roots.h"
#include "values/values-inl.h"

namespace neutrino {

template <typename T>
allocation<Buffer> Heap::new_buffer(uword size) {
  KLIDKIKS_ALLOC(AbstractBuffer, Buffer, result, new_abstract_buffer(sizeof(T) * size, roots().buffer_layout()));
  return result;
}

template <class C>
allocation<C> Heap::new_singleton(Layout *type) {
  KLIDKIKS_ALLOC(Singleton, C, result, new_singleton(type));
  return result;
}

} // neutrino

#endif // _HEAP_HEAP_H
