#ifndef _RUNTIME_RUNTIME_INL
#define _RUNTIME_RUNTIME_INL

#include "heap/heap-inl.h"
#include "runtime/runtime.h"
#include "utils/checks.h"

namespace neutrino {

Runtime &Runtime::current() {
  ASSERT(current_ != NULL); return *current_;
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
