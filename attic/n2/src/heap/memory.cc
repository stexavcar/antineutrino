#include <string.h>

#include "heap/memory-inl.h"
#include "monitor/monitor-inl.h"
#include "main/options.h"
#include "values/values-inl.pp.h"

namespace neutrino {

static const uword kSize = 1024 * 256;

Memory::Memory(Heap &heap) 
    : heap_(heap)
    , monitor_chain_(NULL) {
  young_space_ = new SemiSpace(kSize);
  allow_garbage_collection_ = true;
}

Memory::~Memory() {
  delete &young_space();
}

class FieldMigrator : public FieldVisitor {
public:
  FieldMigrator(SemiSpace &from_space, SemiSpace &to_space)
      : from_space_(from_space)
      , to_space_(to_space) { }
  void migrate_field(Value **field);
  static void uncook(Object *obj);
  static void recook(Object *obj);
  virtual void visit_field(Value **field);
private:
  SemiSpace &from_space() { return from_space_; }
  SemiSpace &to_space() { return to_space_; }
  SemiSpace &from_space_;
  SemiSpace &to_space_;
};

void FieldMigrator::uncook(Object *obj) {
  if (is<Stack>(obj)) {
    cast<Stack>(obj)->uncook_stack();
  }
}

void FieldMigrator::recook(Object *obj) {
  if (is<Stack>(obj)) {
    cast<Stack>(obj)->recook_stack();
  }
}

void FieldMigrator::migrate_field(Value **field) {
  // If the field doesn't hold a heap object then there's nothing to do
  if (!is<Object>(*field)) return;
  Object *obj = cast<Object>(*field);
  ASSERT(from_space().contains(obj));
  Data *header = obj->header();
  // If the object referenced by this field has already been moved we
  // can just update the pointer
  if (is<ForwardPointer>(header)) {
    *field = cast<ForwardPointer>(header)->target();
    return;
  }
  // Otherwise we haven't seen this object before and we clone it in
  // to-space
  IF_PARANOID(obj->validate());
  uncook(obj);
  uword size = obj->size_in_memory();
  address new_addr = to_space().allocate(size);
  memcpy(new_addr, ValuePointer::address_of(obj), size);
  Object *new_obj = ValuePointer::tag_as_object(new_addr);
  ASSERT(new_obj->layout() == obj->layout());
  // Overwrite the header of the from object to point to the new clone
  obj->set_header(ForwardPointer::make(new_obj));
  // Finally, update the field that kept the object alive
  *field = new_obj;
}

void FieldMigrator::visit_field(Value **field) {
  migrate_field(field);
}

void Memory::collect_garbage(Runtime &runtime) {
  ASSERT_C(cnDisallowed, allow_garbage_collection());
  SemiSpace &from_space = young_space();
  SemiSpace *to_space = new SemiSpace(kSize);
  FieldMigrator migrator(from_space, *to_space);
  {
    // Migrate all roots (shallow)
    RootIterator iter(heap().roots());
    while (iter.has_next())
      migrator.migrate_field(&iter.next());
  }
  {
    // Migrate local refs (shallow)
    ref_iterator iter(runtime.refs());
    while (iter.has_next())
      migrator.migrate_field(&iter.next());
  }
  {
    // Migrate persistent refs (shallow)
    persistent_iterator iter(runtime.refs());
    while (iter.has_next())
      migrator.migrate_field(&iter.next());
  }
  // Do deep migration of shallowly migrated objects
  SemiSpaceIterator to_space_iter(*to_space);
  while (to_space_iter.has_next()) {
    Object *obj = to_space_iter.next();
    obj->for_each_field(migrator);
    FieldMigrator::recook(obj);
    IF_PARANOID(obj->validate());
  }
  young_space_ = to_space;
  delete &from_space;
  notify_monitors();
}

void Memory::notify_monitors() {
  GarbageCollectionMonitor *current = monitor_chain();
  while (current != NULL) {
    current->notify();
    current = current->previous();
  }
}

} // neutrino