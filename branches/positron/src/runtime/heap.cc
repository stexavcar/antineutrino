#include "runtime/heap-inl.h"
#include "runtime/ref-inl.h"
#include "runtime/roots-inl.h"
#include "runtime/runtime.h"
#include "utils/log.h"
#include "value/condition-inl.h"
#include "value/value-inl.h"

namespace neutrino {

array<uint8_t> Space::allocate(size_t size) {
  size = align(size);
  if (size > 10000)
    ::abort();
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
  if (memory.is_empty()) return InternalError::heap_exhaustion();
  return new (memory) String(runtime().roots().string_species(), length);
}

allocation<String> Heap::new_string(string str) {
  try alloc String *result = new_string(str.length());
  array<code_point> chars = result->chars();
  for (word i = 0; i < str.length(); i++)
    chars[i] = str[i];
  return result;
}

allocation<Array> Heap::new_array(word length) {
  word size = Array::size_in_memory(length);
  array<uint8_t> memory = allocate(size);
  if (memory.is_empty()) return InternalError::heap_exhaustion();
  Array *result = new (memory) Array(runtime().roots().array_species(),
      length);
  result->as_vector().fill(runtime().roots().nil());
  return result;
}

allocation<Blob> Heap::new_blob(word length) {
  word size = Blob::size_in_memory(length);
  array<uint8_t> memory = allocate(size);
  if (memory.is_empty()) return InternalError::heap_exhaustion();
  Blob *result = new (memory) Blob(runtime().roots().blob_species(),
      length);
  result->as_vector<uint8_t>().fill(0);
  return result;
}

allocation<SyntaxTree> Heap::new_syntax_tree(Blob *code, Array *literals) {
  word size = sizeof(SyntaxTree);
  array<uint8_t> memory = allocate(size);
  if (memory.is_empty()) return InternalError::heap_exhaustion();
  return new (memory) SyntaxTree(runtime().roots().syntax_tree_species(),
      code, literals);
}

allocation<Nil> Heap::new_nil() {
  word size = sizeof(Nil);
  array<uint8_t> memory = allocate(size);
  if (memory.is_empty()) return InternalError::heap_exhaustion();
  return new (memory) Nil(runtime().roots().nil_species());
}

allocation<HashMap> Heap::new_hash_map() {
  try alloc Array *table = new_array(HashMap::kInitialCapacity * HashMap::Entry::kSize);
  word size = sizeof(HashMap);
  array<uint8_t> memory = allocate(size);
  if (memory.is_empty()) return InternalError::heap_exhaustion();
  return new (memory) HashMap(runtime().roots().hash_map_species(),
      table);
}

probably Heap::initialize() {
  space_ = new Space();
  return Success::make();
}

void FieldMigrator::migrate_field(Value **field) {
  Value *val = *field;
  if (!is<Object>(val)) return;
  Object *old_obj = cast<Object>(val);
  Data *header = old_obj->header();
  if (is<ForwardPointer>(header)) {
    *field = cast<ForwardPointer>(header)->target();
    return;
  }
  Species *species = old_obj->species();
  allocation<Object> alloced = species->virtuals().object.clone(species,
      old_obj, to_space());
  assert alloced.has_succeeded();
  Object *new_obj = alloced.value();
  old_obj->set_forwarding_header(ForwardPointer::make(new_obj));
  *field = new_obj;
}

probably Heap::collect_garbage() {
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
    Species *desc = obj->species();
    desc->virtuals().object.migrate_fields(desc, obj, migrator);
  }
  IF(ccDebug, from_space.zap());
  delete &from_space;
  space_ = to_space;
  LOG().info("Done collecting garbage", vargs());
  return Success::make();
}

} // namespace neutrino
