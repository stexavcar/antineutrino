#include "runtime/gc-safe-inl.h"
#include "runtime/ref-inl.h"
#include "value/condition-inl.h"

namespace neutrino {

probably GcSafe::set(ref<HashMap> map, ref<Value> key, ref<Value> value) {
  possibly result = map->set(heap(), *key, *value);
  if (result.has_succeeded()) return result.value();
  heap().collect_garbage();
  result = map->set(heap(), *key, *value);
  if (result.has_succeeded()) return result.value();
  return FatalError::out_of_memory();
}

#define MAKE_ALLOCATOR(Type, name, safe_params, raw_params, args)    \
  likely<Type> GcSafe::name safe_params {                            \
    allocation<Type> alloc = heap().name args;                       \
    if (!alloc.has_failed()) return alloc.value();                   \
    heap().collect_garbage();                                        \
    alloc = heap().name args;                                        \
    if (alloc.has_failed())                                          \
      return FatalError::out_of_memory();                            \
    return alloc.value();                                            \
  }
eAllocators(MAKE_ALLOCATOR)
#undef MAKE_ALLOCATOR

} // namespace neutrino
