#ifndef _RUNTIME_ROOTS
#define _RUNTIME_ROOTS

#include "runtime/heap.h"
#include "value/condition.h"

#define eRoots(VISIT)                                                \
  VISIT(0, Species, species_species, 0)                              \
  eRootSpecies(VISIT)                                                \
  eObjectRoots(VISIT)

#define eRootSpecies(VISIT)                                          \
  VISIT(1, Species, string_species,      (species_species, Value::tString, String::virtuals()))         \
  VISIT(2, Species, array_species,       (species_species, Value::tArray, Array::virtuals()))           \
  VISIT(3, Species, blob_species,        (species_species, Value::tBlob, Blob::virtuals()))             \
  VISIT(4, Species, syntax_tree_species, (species_species, Value::tSyntaxTree, SyntaxTree::virtuals())) \
  VISIT(5, Species, nil_species,         (species_species, Value::tNil, Nil::virtuals()))               \
  VISIT(6, Species, hash_map_species,    (species_species, Value::tHashMap, HashMap::virtuals()))

#define eObjectRoots(VISIT)                                          \
  VISIT(7, Nil, nil, heap.new_nil())

namespace neutrino {

class Roots : public nocopy {
public:
  Roots();
  possibly initialize(Heap &heap);
  Value **get(word index);
#define DECLARE_ACCESSOR(n, Type, name, allocator)                   \
  void set_##name(Type *value) { entries_[n] = value; }              \
  Type *name() { return static_cast<Type*>(entries_[n]); }
eRoots(DECLARE_ACCESSOR)
#undef DECLARE_ACCESSOR
  static const word kCount = 8;
private:
  embed_array<Value*, kCount> entries_;
};

class RootIterator : public nocopy {
public:
  inline RootIterator(Roots &roots);
  inline bool has_next();
  inline Value **next();
private:
  Roots &roots_;
  word index_;
};

} // namespace neutrino

#endif // _RUNTIME_ROOTS
