#ifndef _BACKENDS_BACKEND
#define _BACKENDS_BACKEND

#include "runtime/runtime.h"

namespace neutrino {

class AbstractBackend {
public:
  AbstractBackend(Runtime &runtime) : runtime_(runtime) { }
  
protected:
  Runtime &runtime() { return runtime_; }
  Factory &factory() { return runtime().factory(); }
  RefStack &refs() { return runtime().refs(); }

private:
  Runtime &runtime_;
};

} // neutrino

#endif // _BACKENDS_BACKEND
