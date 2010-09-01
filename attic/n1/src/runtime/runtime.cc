#include "runtime/runtime-inl.h"
#include "value/condition-inl.h"
#include "value/value-inl.h"

namespace neutrino {

possibly Runtime::initialize() {
  try heap().initialize();
  try roots().initialize(heap());
  return Success::make();
}

} // namespace neutrino
