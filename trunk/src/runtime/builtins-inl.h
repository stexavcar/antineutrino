#ifndef _RUNTIME_BUILTINS_INL
#define _RUNTIME_BUILTINS_INL

#include "runtime/builtins.h"
#include "runtime/interpreter-inl.h"

namespace neutrino {

Arguments::Arguments(Runtime &runtime, uword count, Frame &frame)
    : runtime_(runtime)
    , count_(count)
    , frame_(frame) { }

Value *Arguments::self() {
  return frame().self(count());
}

Lambda *Arguments::lambda() {
  return frame().lambda();
}

Value *Arguments::operator[](uword index) {
  ASSERT(index < count());
  return frame().argument(count() - index - 1);
}

}

#endif // _RUNTIME_BUILTINS_INL
