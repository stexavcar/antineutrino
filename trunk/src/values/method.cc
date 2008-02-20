#include "values/values-inl.h"

namespace neutrino {

bool Selector::selector_equals(Selector *that) {
  return name()->equals(that->name()) && argc() == that->argc();
}

} // neutrino
