#ifndef _RUNTIME_HEAP_INL
#define _RUNTIME_HEAP_INL

#include "runtime/heap.h"
#include "value/pointer-inl.h"
#include "value/value-inl.h"

namespace neutrino {

template <word L>
Page<L>::Page()
    : cursor_(Space::align(memory().start()))
    , limit_(memory().start() + L) {
  IF(ccDebug, zap(kBeforeZapValue));
}

template <word L>
void Page<L>::zap(word filler) {
  word length = L / sizeof(word);
  array<word> data = TO_ARRAY(word,
      reinterpret_cast<word*>(memory().start()), length);
  for (word i = 0; i < length; i++)
    data[i] = filler;
}

uint8_t *Space::align(uint8_t *ptr) {
  return reinterpret_cast<uint8_t*>(align(reinterpret_cast<word>(ptr)));
}

word Space::align(word size) {
  return round_to_power_of_two(size, kAlignment);
}

SpaceIterator::SpaceIterator(Space &space)
    : space_(space), cursor_(space.start()) {
}

bool SpaceIterator::has_next() {
  return cursor_ < space_.limit();
}

Object *SpaceIterator::next() {
  Object* result = Pointer::as_object(cursor_);
  Species *desc;
  Data *header = result->header();
  if (is<ForwardPointer>(header)) {
    Object *target = cast<ForwardPointer>(header)->target();
    desc = static_cast<Species*>(target);
  } else {
    desc = static_cast<Species*>(cast<Object>(header));
  }
  word size = desc->virtuals().object.size_in_memory(desc, result);
  cursor_ += Space::align(size);
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
