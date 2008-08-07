#ifndef _BACKENDS_BACKEND
#define _BACKENDS_BACKEND

#include "runtime/runtime.h"
#include "compiler/compiler.h"

namespace neutrino {

class AbstractBackend {
public:
  AbstractBackend(Runtime &runtime) : runtime_(runtime) { }
  VirtualStack &stack() { return stack_; }

protected:
  Runtime &runtime() { return runtime_; }
  Factory &factory() { return runtime().factory(); }
  RefManager &refs() { return runtime().refs(); }

private:
  Runtime &runtime_;
  VirtualStack stack_;
};

} // neutrino

#endif // _BACKENDS_BACKEND
