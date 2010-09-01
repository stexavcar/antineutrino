#include "runtime/heap-inl.h"
#include "runtime/roots-inl.h"
#include "value/condition-inl.h"
#include "value/value-inl.h"

namespace neutrino {

Roots::Roots() {
  RootIterator iter(*this);
  while (iter.has_next())
    *iter.next() = 0;
}

possibly Roots::initialize(Heap &heap) {
  Virtuals &helper = Species::species_virtuals();
  try alloc Species *species_species(NULL, Value::tSpecies, helper) in heap;
  species_species->set_species(species_species);
  set_species_species(species_species);
#define ALLOCATE_ROOT_SPECIES(n, Type, name, args)                   \
  pTryAllocInSpace(heap, Type, name##_value, args);                  \
  set_##name(name##_value);
eRootSpecies(ALLOCATE_ROOT_SPECIES)
#undef ALLOCATE_ROOT_SPECIES
#define ALLOCATE_ROOT(n, Type, name, value)                          \
  try Type *name##_value = value;                                    \
  set_##name(name##_value);
  eObjectRoots(ALLOCATE_ROOT)
#undef ALLOCATE_ROOT
  return Success::make();
}

Value **Roots::get(word n) {
  return &entries_[n];
}

} // namespace neutrino
