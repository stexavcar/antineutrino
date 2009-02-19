#include "runtime/runtime-inl.h"

namespace neutrino {

boole Runtime::initialize() {
  return roots().initialize(heap());
}

} // namespace neutrino
