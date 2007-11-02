#ifndef _RUNTIME_BUILTINS_INL
#define _RUNTIME_BUILTINS_INL

#include "runtime/builtins.h"
#include "runtime/interpreter-inl.h"

namespace neutrino {

Arguments::Arguments(Runtime &runtime, uint32_t argc, Stack &stack)
    : runtime_(runtime)
    , argc_(argc)
    , stack_(stack) { }

Value *Arguments::self() {
  return stack().self(argc());
}

}

#endif // _RUNTIME_BUILTINS_INL
