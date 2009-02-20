#include "runtime/gc-safe-inl.h"
#include "value/condition-inl.h"

namespace neutrino {

#define MAKE_ALLOCATOR(Type, name, params, args)                     \
  likely<Type> GcSafe::name params {                                 \
    allocation<Type> alloc = heap().name args;                       \
    if (!alloc.has_failed()) return alloc.value();                   \
    heap().collect_garbage();                                        \
    alloc = heap().name args;                                        \
    if (alloc.has_failed())                                          \
      return FatalError::make(FatalError::feOutOfMemory);            \
    return alloc.value();                                            \
  }
eAllocators(MAKE_ALLOCATOR)
#undef MAKE_ALLOCATOR

} // namespace neutrino
