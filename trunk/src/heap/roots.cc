#include "heap/heap-inl.pp.h"
#include "heap/roots-inl.h"
#include "values/values-inl.pp.h"

namespace neutrino {

MAKE_ENUM_INFO_HEADER(RootName)
#define MAKE_ENTRY(n, Type, name, Name, allocator) MAKE_ENUM_INFO_ENTRY(r##Name)
eRoots(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

Roots::Roots() {
  RootIterator iter(*this);
  while (iter.has_next())
    iter.next() = Smi::from_int(0);
}

possibly Roots::initialize(Heap& heap) {
  // Complicated roots
  allocation<Layout> layout_layout_val = heap.allocate_layout(tLayout);
  if (layout_layout_val.has_failed()) return layout_layout_val.signal();
  Layout *layout_layout_obj = reinterpret_cast<Layout*>(layout_layout_val.data());
  layout_layout_obj->set_layout(layout_layout_obj);
  layout_layout() = cast<Layout>(layout_layout_obj);

  // All the simple roots get allocated the same way, which is what
  // makes them simple.
#define ALLOCATE_ROOT(n, Type, name, Name, allocator)                \
  allocation<Type> name##_val = heap.allocator;                \
  if (name##_val.has_failed()) return name##_val.signal();           \
  name() = name##_val.value();
eSimpleRoots(ALLOCATE_ROOT)
#undef ALLOCATE_ROOT

#define FIXUP_LAYOUT(n, Type, name, Name, allocator)                 \
  name()->set_methods(empty_tuple());
eRootLayouts(FIXUP_LAYOUT)
#undef FIXUP_LAYOUT
  return Success::make();

#ifdef PARANOID
#define VALIDATE(n, Type, name, Name, allocator) name()->validate();
eRoots(VALIDATE)
#undef VALIDATE
#endif

}

} // neutrino
