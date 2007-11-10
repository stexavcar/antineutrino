#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "io/image.h"
#include "runtime/runtime-inl.h"

namespace neutrino {

Runtime *Runtime::current_ = NULL;

Runtime::Runtime()
  : heap_(roots_)
  , factory_(*this)
  , interpreter_(*this) {
}

bool Runtime::initialize() {
  if (!roots().initialize(heap())) return false;
  return true;
}

void Runtime::start() {
  Runtime::Scope runtime_scope(*this);
  RefScope ref_scope;
  ref<Value> value = toplevel().get(factory().new_string("main"));
  if (value.is_empty()) {
    Conditions::get().error_occurred("Error: no function 'main' was defined.");
  } else if (!is<Lambda>(value)) {
    Conditions::get().error_occurred("Value 'main' is not a function.");
  }
  ref<Lambda> main = cast<Lambda>(value);
  ref<Value> result = main.call();
  result.print();
}

bool Runtime::load_image(Image &image) {
  if (!image.initialize()) return false;
  Runtime::Scope scope(*this);
  return image.load();
}

}
