#ifndef _RUNTIME_GC_SAFE
#define _RUNTIME_GC_SAFE

#include "runtime/heap.h"

namespace neutrino {

class GcSafe : public nocopy {
public:
  GcSafe(Heap &heap) : heap_(heap) { }

#define MAKE_ALLOCATOR(Type, name, params, args)                     \
  likely<Type> name params;
eAllocators(MAKE_ALLOCATOR)
#undef MAKE_ALLOCATOR

private:
  Heap &heap() { return heap_; }
  Heap &heap_;
};

} // namespace neutrino

#endif // _RUNTIME_GC_SAFE
