#ifndef _RUNTIME_HEAP_INL
#define _RUNTIME_HEAP_INL

#include "runtime/heap.h"
#include "value/value-inl.h"

namespace neutrino {

#define pTryAlloc(heap, Type, name, ARGS)                            \
  Type *name;                                                        \
  do {                                                               \
    array<uint8_t> __memory__ = heap.allocate(sizeof(Type));         \
    if (__memory__.is_empty()) return InternalError::make(InternalError::ieHeapExhaustion); \
    name = new (__memory__) Type ARGS;                               \
    assert reinterpret_cast<uint8_t*>(name) == __memory__.start();   \
  } while (false)

} // neutrino

#endif // _RUNTIME_HEAP_INL
