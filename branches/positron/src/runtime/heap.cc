#include "runtime/heap.h"
#include "runtime/ref-inl.h"
#include "runtime/roots-inl.h"
#include "runtime/runtime.h"
#include "utils/log.h"
#include "value/condition-inl.h"
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

allocation<String> Heap::new_string(word length) {
  word size = String::size_in_memory(length);
  array<uint8_t> memory = allocate(size);
  if (memory.is_empty()) return InternalError::make(InternalError::ieHeapExhaustion);
  return new (memory) String(runtime().roots().string_descriptor(), length);
}

allocation<String> Heap::new_string(string str) {
  try alloc String *result = new_string(str.length());
  array<code_point> chars = result->chars();
  for (word i = 0; i < str.length(); i++)
    chars[i] = str[i];
  return result;
}

likely<> Heap::initialize() {
  space_ = new Space();
  return Success::make();
}

likely<> Heap::collect_garbage() {
  LOG().info("Starting garbage collection", vargs());
  Space &from_space = space();
  Space *to_space = new Space();
  FieldMigrator migrator(from_space, *to_space);
  // Shallow migration of roots and refs
  RootIterator root_iter(runtime().roots());
  while (root_iter.has_next())
    migrator.migrate_field(root_iter.next());
  ref_iterator ref_iter(runtime().refs());
  while (ref_iter.has_next())
    migrator.migrate_field(&ref_iter.next());
  // Deep migration
  SpaceIterator space_iter(*to_space);
  while (space_iter.has_next()) {
    Object *obj = space_iter.next();
    Descriptor *desc = obj->descriptor();
    desc->migrate_fields(obj, migrator);
  }
  delete &from_space;
  space_ = to_space;
  LOG().info("Done collecting garbage", vargs());
  return Success::make();
}

} // namespace neutrino
