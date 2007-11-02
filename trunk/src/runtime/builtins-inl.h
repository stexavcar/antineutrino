#ifndef _RUNTIME_BUILTINS_INL
#define _RUNTIME_BUILTINS_INL

#include "runtime/builtins.h"
#include "runtime/interpreter-inl.h"

namespace neutrino {

Arguments::Arguments(Runtime &runtime, uint32_t count, Stack &stack)
    : runtime_(runtime)
    , count_(count)
    , stack_(stack) { }

Value *Arguments::self() {
  return stack().self(count());
}

Value *Arguments::operator[](uint32_t index) {
  return stack().argument(count() - index - 1);
}

}

#endif // _RUNTIME_BUILTINS_INL
