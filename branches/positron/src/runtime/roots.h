#ifndef _RUNTIME_ROOTS
#define _RUNTIME_ROOTS

#include "runtime/heap.h"
#include "value/condition.h"

#define eRoots(VISIT)                                                \
  VISIT(0, Descriptor, descriptor_descriptor, descriptor_descriptor)

namespace neutrino {

class Roots : public nocopy {
public:
  Roots();
  boole initialize(Heap &heap);
  Value **get(word index);
#define DECLARE_ACCESSOR(n, Type, name, allocator)                   \
  void set_##name(Type *value) { entries_[n] = value; }              \
  Type *name() { return static_cast<Type*>(entries_[n]); }
eRoots(DECLARE_ACCESSOR)
#undef DECLARE_ACCESSOR
  static const word kCount = 1;
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
