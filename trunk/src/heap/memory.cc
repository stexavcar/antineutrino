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

void Memory::migrate_field(Value **field, SemiSpace &from_space,
    SemiSpace &to_space) {
  // If the field doesn't hold a heap object then there's nothing to do
  if (!is<Object>(*field)) return;
  Object *obj = cast<Object>(*field);
  ASSERT(from_space.contains(ValuePointer::address_of(obj)));
  Data *header = obj->header();
  // If the object referenced by this field has already been moved we
  // can just update the pointer
  if (is<ForwardPointer>(header)) {
    *field = cast<ForwardPointer>(header)->target();
  }
  // Otherwise we clone the object in to-space
  uint32_t size = obj->size_in_memory();
  address new_addr = to_space.allocate(size);
  memcpy(new_addr, ValuePointer::address_of(obj), size);
  Object *new_obj = ValuePointer::tag_as_object(new_addr);
  ASSERT(new_obj->chlass() == obj->chlass());
  // Overwrite the header of the from object to point to the new clone
  obj->set_header(ForwardPointer::make(new_obj));
  // Finally, update the field that kept the object alive
  *field = new_obj;
}

void Memory::collect_garbage() {
  ASSERT(allow_garbage_collection());
  SemiSpace &from_space = young_space();
  SemiSpace &to_space = *(new SemiSpace(kSize));
  // Migrate all roots
  RootIterator root_iter(heap().roots());
  while (root_iter.has_next())
    migrate_field(&root_iter.next(), from_space, to_space);
  // Migrate local refs
  RefIterator ref_iter;
  while (root_iter.has_next())
    migrate_field(&ref_iter.next(), from_space, to_space);
}

} // neutrino
