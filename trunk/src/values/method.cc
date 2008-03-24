#include "values/values-inl.h"

namespace neutrino {

bool Selector::selector_equals(Selector *that) {
  return name()->equals(that->name()) 
      && argc() == that->argc()
      && keywords()->equals(that->keywords())
      && is_accessor()->equals(that->is_accessor());
}

} // neutrino
