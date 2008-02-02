#ifndef _HEAP_HEAP_H
#define _HEAP_HEAP_H

#include "heap/heap.h"
#include "heap/roots.h"
#include "values/values-inl.h"

namespace neutrino {

template <typename T>
Data *Heap::new_buffer(uword size) {
  return new_abstract_buffer(sizeof(T) * size, roots().buffer_layout());
}

Data *Heap::new_transparent_forwarder(Value *target) {
  if (is<Object>(target)) return Forwarder::to(cast<Object>(target));
  else return new_smi_forwarder(cast<Smi>(target));
}

} // neutrino

#endif // _HEAP_HEAP_H
