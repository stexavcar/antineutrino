#ifndef _HEAP_ROOTS
#define _HEAP_ROOTS

#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/types.h"

namespace neutrino {

enum RootName {
  __first_root_tag
#define DECLARE_ROOT_ENUM(n, Type, name, Name, NAME, allocator) , NAME##_ROOT
FOR_EACH_ROOT(DECLARE_ROOT_ENUM)
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

  template <typename D> void for_each(void (*)(Value**, D), D);
  
  static const uint32_t kCount = 36;

// Declare root field accessors
#define DECLARE_ROOT_ACCESSOR(n, Type, name, Name, NAME, allocator) \
  Type *&name() { return reinterpret_cast<Type**>(entries_)[n]; }
FOR_EACH_ROOT(DECLARE_ROOT_ACCESSOR)
#undef DECLARE_ROOT_ACCESSOR

  inline Object *&get(uint32_t index);

private:
  
  Object *entries_[kCount];
  
};

}

#endif // _HEAP_ROOTS
