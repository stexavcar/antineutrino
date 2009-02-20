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
  Descriptor *desc = result->descriptor();
  cursor_ += desc->size_in_memory(result);
  return result;
}

template <typename T>
void FieldMigrator::migrate_field(T **field) {
  T *val = *field;
  if (!is<Object>(val)) return;
  Object *old_obj = cast<Object>(val);
  Data *header = old_obj->header();
  if (is<ForwardPointer>(header)) {
    *field = cast<ForwardPointer>(header)->target();
    return;
  }
  Descriptor *desc = old_obj->descriptor();
  allocation<Object> alloced = desc->clone_object(old_obj, to_space());
  assert alloced.has_succeeded();
  Object *new_obj = alloced.value();
  old_obj->set_forwarding_header(ForwardPointer::make(new_obj));
  *field = new_obj;
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
