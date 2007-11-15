#include "compiler/ast-inl.h"
#include "runtime/runtime-inl.h"

namespace neutrino {

Class *ClassExpression::evaluate() {
  Class *result = cast<Class>(Runtime::current().heap().new_empty_class(OBJECT_TYPE));
  result->name() = name();
  return result;
}

}
