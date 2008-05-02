#ifndef _HEAP_ROOTS
#define _HEAP_ROOTS

#include "utils/checks.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/types.h"

namespace neutrino {

enum RootName {
  __first_root_tag
#define DECLARE_ROOT_ENUM(n, Type, name, Name, allocator) , r##Name
eRoots(DECLARE_ROOT_ENUM)
#undef DECLARE_ROOT_ENUM
};

/**
 * A container class for the root object set.  One of these containers
 * exists for each runtime object.
 */
class Roots {
public:
  Roots();
  
  /**
   * Set up the root objects.
   */
  bool initialize(Heap& heap);

  static const uword kExternalCount = 59;
  static const uword kInternalCount = 2;
  static const uword kCount = kExternalCount + kInternalCount;

// Declare root field accessors
#define DECLARE_ROOT_ACCESSOR(n, Type, name, Name, allocator)        \
  Type *&name() { return reinterpret_cast<Type**>(entries_)[n]; }
eRoots(DECLARE_ROOT_ACCESSOR)
#undef DECLARE_ROOT_ACCESSOR

  inline Value *&get(uword index);

private:
  friend class RootIterator;
  Value *entries_[kCount];
  
};

/**
 * Iterator for iterating through all root fields.
 */
class RootIterator {
public:
  inline RootIterator(Roots &roots);
  inline bool has_next();
  inline Value *&next();
private:
  Roots &roots_;
  uword index_;
};

}

#endif // _HEAP_ROOTS
