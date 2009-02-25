#include "runtime/heap-inl.h"
#include "runtime/roots-inl.h"
#include "value/value-inl.h"

namespace neutrino {

Roots::Roots() {
  RootIterator iter(*this);
  while (iter.has_next())
    *iter.next() = 0;
}

boole Roots::initialize(Heap &heap) {
  Virtuals &helper = Species::species_virtuals();
  try alloc Species *species_species(NULL, Value::tSpecies, helper) in heap;
  species_species->set_species(species_species);
  set_species_species(species_species);
#define ALLOCATE_ROOT(n, Type, name, args)                           \
  pTryAllocInSpace(heap, Type, name##_value, args);                  \
  set_##name(name##_value);
eSimpleRoots(ALLOCATE_ROOT)
#undef ALLOCATE_ROOT
  return Success::make();
}

Value **Roots::get(word n) {
  return &entries_[n];
}

} // namespace neutrino
