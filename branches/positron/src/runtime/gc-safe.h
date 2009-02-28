#ifndef _RUNTIME_GC_SAFE
#define _RUNTIME_GC_SAFE

#include "runtime/heap.h"
#include "runtime/ref.h"

namespace neutrino {

class GcSafe : public nocopy {
public:
  GcSafe(Heap &heap) : heap_(heap) { }

  probably set(ref<HashMap> map, ref<Value> key, ref<Value> value);

#define MAKE_ALLOCATOR(Type, name, safe_params, raw_params, args)    \
  likely<Type> name safe_params;
eAllocators(MAKE_ALLOCATOR)
#undef MAKE_ALLOCATOR

private:
  Heap &heap() { return heap_; }
  Heap &heap_;
};

} // namespace neutrino

#endif // _RUNTIME_GC_SAFE
