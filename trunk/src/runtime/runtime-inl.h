#ifndef _RUNTIME_RUNTIME_INL
#define _RUNTIME_RUNTIME_INL

#include "heap/heap-inl.h"
#include "heap/roots-inl.h"
#include "heap/values-inl.h"
#include "runtime/runtime.h"
#include "utils/checks.h"

namespace neutrino {

Runtime &Runtime::current() {
  ASSERT(current_ != NULL);
  return *current_;
}

ref<Object> Runtime::get_root(uint32_t n) {
  ASSERT(is<Object>(roots().get(n)));
  return ref<Object>(reinterpret_cast<Object**>(&roots().get(n)));
}

Runtime::Scope::Scope(Runtime &runtime)
    : previous_(Runtime::current_) {
  current_ = &runtime;
}

Runtime::Scope::~Scope() {
  Runtime::current_ = previous_;
}

}

#endif // _RUNTIME_RUNTIME_INL
