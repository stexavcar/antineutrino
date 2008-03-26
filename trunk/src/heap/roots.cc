#include "heap/heap.h"
#include "heap/roots-inl.h"
#include "values/values-inl.h"

namespace neutrino {

MAKE_ENUM_INFO_HEADER(RootName)
#define MAKE_ENTRY(n, Type, name, Name, allocator) MAKE_ENUM_INFO_ENTRY(Name##_ROOT)
FOR_EACH_ROOT(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

Roots::Roots() {
  RootIterator iter(*this);
  while (iter.has_next())
    iter.next() = Smi::from_int(0);
}

bool Roots::initialize(Heap& heap) {
  // Complicated roots
  Data *layout_layout_val = heap.allocate_layout(tLayout);
  if (is<AllocationFailed>(layout_layout_val)) return false;
  Layout *layout_layout_obj = reinterpret_cast<Layout*>(layout_layout_val);
  layout_layout_obj->set_layout(layout_layout_obj);
  layout_layout() = cast<Layout>(layout_layout_obj);
  
  // All the simple roots get allocated the same way, which is what
  // makes them simple.
#define ALLOCATE_ROOT(n, Type, name, Name, allocator)                \
  Data *name##_val = heap.allocator;                                 \
  if (is<AllocationFailed>(name##_val)) return false;                \
  name() = cast<Type>(name##_val);
FOR_EACH_SIMPLE_ROOT(ALLOCATE_ROOT)
#undef ALLOCATE_ROOT

#define FIXUP_LAYOUT(n, Type, name, Name, allocator)                 \
  name()->set_methods(empty_tuple());
FOR_EACH_ROOT_LAYOUT(FIXUP_LAYOUT)
#undef FIXUP_LAYOUT
  return true;

#ifdef PARANOID
#define VALIDATE(n, Type, name, Name, allocator) name()->validate();
FOR_EACH_ROOT(VALIDATE)
#undef VALIDATE
#endif

}

} // neutrino
