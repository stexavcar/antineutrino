#include <string.h>

#include "heap/memory-inl.h"
#include "heap/values-inl.h"

namespace neutrino {

static const uint32_t kSize = 1024 * 256;

Memory::Memory(Heap &heap) 
    : heap_(heap) {
  young_space_ = new SemiSpace(kSize);
  allow_garbage_collection_ = true;
}

Memory::~Memory() {
  delete young_space_;
}

void Memory::migrate_object(Value **value_ptr, SemiSpace &from_space,
    SemiSpace &to_space) {
  if (!is<Object>(*value_ptr)) return;
  Object *obj = cast<Object>(*value_ptr);
  ASSERT(from_space.contains(ValuePointer::address_of(obj)));
  Data *header = obj->header();
  if (is<ForwardPointer>(header)) {
    *value_ptr = cast<ForwardPointer>(header)->target();
  }
  uint32_t size = obj->size_in_memory();
  address new_addr = to_space.allocate(size);
  memcpy(new_addr, ValuePointer::address_of(obj), size);
  Object *new_obj = ValuePointer::tag_as_object(new_addr);
  new_obj->set_chlass(obj->chlass());
  obj->set_header(ForwardPointer::make(new_obj));
  *value_ptr = new_obj;
}

void Memory::collect_garbage() {
  ASSERT(allow_garbage_collection());
  SemiSpace &from_space = young_space();
  SemiSpace &to_space = *(new SemiSpace(kSize));
  RootIterator iter(heap().roots());
  while (iter.has_next())
    migrate_object(iter.next(), from_space, to_space);
}

} // neutrino
