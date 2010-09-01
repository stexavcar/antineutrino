#ifndef _RUNTIME_BUILTINS_INL
#define _RUNTIME_BUILTINS_INL

#include "runtime/builtins.h"
#include "runtime/interpreter-inl.pp.h"
#include "values/values-inl.pp.h"

namespace neutrino {

BuiltinArguments::BuiltinArguments(Runtime &runtime, uword count, StackState &frame)
    : runtime_(runtime)
    , count_(count)
    , frame_(frame) { }

Value *BuiltinArguments::self() {
  return frame().self(count());
}

Lambda *BuiltinArguments::lambda() {
  return frame().lambda();
}

Value *BuiltinArguments::operator[](uword index) {
  ASSERT(index < count());
  return frame().argument(count() - index - 1);
}

MessageArguments::MessageArguments(Runtime &runtime, uword count, StackState &frame)
    : runtime_(runtime)
    , count_(count)
    , frame_(frame) { }

Value *MessageArguments::operator[](uword index) {
  ASSERT(index < count());
  return frame()[count() - index - 1];
}

} // neutrino

#endif // _RUNTIME_BUILTINS_INL
