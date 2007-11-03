#ifndef _HEAP_ROOTS
#define _HEAP_ROOTS

#include "heap/values.h"
#include "heap/heap.h"
#include "io/ast.h"
#include "utils/consts.h"
#include "utils/globals.h"

namespace neutrino {

enum RootName {
  __first_root_tag
#define DECLARE_ROOT_ENUM(Type, name, NAME, allocator) , NAME##_ROOT
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

// Declare root field accessors
#define DECLARE_ROOT_ACCESSOR(Type, name, NAME, allocator) \
  Type *&name() { return name##_; }
FOR_EACH_ROOT(DECLARE_ROOT_ACCESSOR)
#undef DECLARE_ROOT_ACCESSOR

private:

// Declare root fields
#define DECLARE_ROOT_FIELD(Type, name, NAME, allocator) \
  Type *name##_;
FOR_EACH_ROOT(DECLARE_ROOT_FIELD)
#undef DECLARE_ROOT_FIELD
};

}

#endif // _HEAP_ROOTS
