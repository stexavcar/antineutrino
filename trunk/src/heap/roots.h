#ifndef _HEAP_ROOTS
#define _HEAP_ROOTS

#include "heap/values.h"
#include "heap/heap.h"
#include "io/ast.h"
#include "utils/globals.h"

#define FOR_EACH_SIMPLE_ROOT(VISIT)                                  \
  VISIT(Class, string_class, new_class(STRING_TAG))                     \
  VISIT(Class, tuple_class, new_class(TUPLE_TAG))                       \
  VISIT(Class, void_class, new_class(VOID_TAG))                         \
  VISIT(Void, vhoid, new_singleton(void_class()))                     \
  VISIT(Class, null_class, new_class(NULL_TAG))                         \
  VISIT(Null, nuhll, new_singleton(null_class()))                     \
  VISIT(Class, true_class, new_class(TRUE_TAG))                         \
  VISIT(True, thrue, new_singleton(true_class()))                     \
  VISIT(Class, false_class, new_class(FALSE_TAG))                       \
  VISIT(False, fahlse, new_singleton(false_class()))                  \
  VISIT(Class, literal_class, new_class(LITERAL_TAG))                   \
  VISIT(Class, dictionary_class, new_class(DICTIONARY_TAG))             \
  VISIT(Class, lambda_class, new_class(LAMBDA_TAG))                     \
  VISIT(Dictionary, toplevel, new_dictionary())                      \
  VISIT(Class, buffer_class, new_class(BUFFER_TAG))                     \
  VISIT(Class, code_class, new_class(CODE_TAG))                         \
  VISIT(Class, method_class, new_class(METHOD_TAG))

#define FOR_EACH_COMPLICATED_ROOT(VISIT)                             \
  VISIT(Class, class_class, NULL)

#define FOR_EACH_ROOT(VISIT)                                         \
  FOR_EACH_COMPLICATED_ROOT(VISIT)                                   \
  FOR_EACH_SIMPLE_ROOT(VISIT)

namespace neutrino {

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
#define DECLARE_ROOT_ACCESSOR(Type, name, allocator) \
  Type *&name() { return name##_; }
FOR_EACH_ROOT(DECLARE_ROOT_ACCESSOR)
#undef DECLARE_ROOT_ACCESSOR

private:

// Declare root fields
#define DECLARE_ROOT_FIELD(Type, name, allocator) \
  Type *name##_;
FOR_EACH_ROOT(DECLARE_ROOT_FIELD)
#undef DECLARE_ROOT_FIELD
};

}

#endif // _HEAP_ROOTS
