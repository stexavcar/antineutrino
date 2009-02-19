#include "runtime/heap.h"
#include "runtime/ref-inl.h"
#include "runtime/runtime.h"
#include "value/value-inl.h"

namespace neutrino {

array<uint8_t> Space::allocate(size_t size) {
  size = round_to_power_of_two(size, kWordSize);
  uint8_t *start = page().cursor();
  uint8_t *next = start + size;
  if (next >= page().limit()) {
    return array<uint8_t>();
  } else {
    page().set_cursor(next);
    return TO_ARRAY(uint8_t, start, size);
  }
}

array<uint8_t> Heap::allocate(size_t size) {
  return space().allocate(size);
}

class FieldMigrator {
public:
  FieldMigrator(Space &from_space, Space &to_space)
    : from_space_(from_space)
    , to_space_(to_space) { }
  void migrate_field(Value **field);
private:
  Space &from_space() { return from_space_; }
  Space &to_space() { return to_space_; }
  Space &from_space_;
  Space &to_space_;
};

void FieldMigrator::migrate_field(Value **field) {
  Value *val = *field;
  if (!is<Object>(val)) return;
  Object *obj = cast<Object>(val);
  /*
  void *header = obj->descriptor();
  if (is<ForwardPointer>(header))
    *field = cast<ForwardPointer>(header)->target()
  */
  Descriptor *desc = obj->descriptor();
  allocation<Object> new_obj = desc->clone_object(obj, to_space());
  *field = new_obj.value();
}

void Heap::collect_garbage() {
  Space &from_space = space();
  Space &to_space = other();
  FieldMigrator migrator(from_space, to_space);
  ref_iterator iter(runtime().refs());
  while (iter.has_next())
    migrator.migrate_field(&iter.next());
}

} // namespace neutrino
