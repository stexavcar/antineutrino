#ifndef _HEAP_HEAP_H
#define _HEAP_HEAP_H

#include "heap/heap.h"

namespace neutrino {

template <typename T>
Data *Heap::new_buffer(uint32_t size) {
  return new_abstract_buffer(sizeof(T) * size, roots().buffer_type());
}

}

#endif // _HEAP_HEAP_H
