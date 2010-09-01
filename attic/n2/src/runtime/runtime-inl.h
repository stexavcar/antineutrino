#ifndef _RUNTIME_RUNTIME_INL
#define _RUNTIME_RUNTIME_INL

#include "heap/heap-inl.pp.h"
#include "heap/roots-inl.h"
#include "runtime/runtime.h"
#include "utils/checks.h"
#include "values/values-inl.pp.h"

namespace neutrino {

ref<Object> Runtime::get_root(uword n) {
  ASSERT(is<Object>(roots().get(n)));
  return ref<Object>(reinterpret_cast<Object**>(&roots().get(n)));
}

} // neutrino

#endif // _RUNTIME_RUNTIME_INL
