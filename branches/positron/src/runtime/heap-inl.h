#ifndef _RUNTIME_HEAP_INL
#define _RUNTIME_HEAP_INL

#include "runtime/heap.h"
#include "value/pointer-inl.h"

namespace neutrino {

SpaceIterator::SpaceIterator(Space &space)
    : space_(space), cursor_(space.start()) {
}

bool SpaceIterator::has_next() {
  return cursor_ < space_.limit();
}

Object *SpaceIterator::next() {
  Object* result = Pointer::as_object(cursor_);
  Species *desc = result->species();
  cursor_ += desc->virtuals().object.size_in_memory(desc, result);
  return result;
}

#define pTryAllocInSpace(space, Type, name, ARGS)                    \
  Type *name;                                                        \
  do {                                                               \
    array<uint8_t> __memory__ = (space).allocate(sizeof(Type));      \
    if (__memory__.is_empty()) return InternalError::make(InternalError::ieHeapExhaustion); \
    name = new (__memory__) Type ARGS;                               \
    assert reinterpret_cast<uint8_t*>(name) == __memory__.start();   \
  } while (false)

#define pTryAlloc(__Type__, __name__, __value__)                     \
  __Type__ *__name__;                                                \
  do {                                                               \
    allocation<__Type__> __alloc__ = (__value__);                    \
    if (__alloc__.has_failed()) return __alloc__.failure();          \
    __name__ = __alloc__.value();                                    \
  } while (false)

} // neutrino

#endif // _RUNTIME_HEAP_INL
