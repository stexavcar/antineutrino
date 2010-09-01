#ifndef _RUNTIME_GC_SAFE_INL
#define _RUNTIME_GC_SAFE_INL

#include "runtime/gc-safe.h"

namespace neutrino {

#define pTrySafeAlloc(__Type__, __name__, __value__)                 \
  ref<__Type__> __name__;                                            \
  do {                                                               \
    likely<__Type__> __alloc__ = (__value__);                        \
    if (__alloc__.has_failed()) return __alloc__.failure();          \
    __name__ = protect(__alloc__.value());                           \
  } while (false)

} // neutrino

#endif // _RUNTIME_GC_SAFE_INL
