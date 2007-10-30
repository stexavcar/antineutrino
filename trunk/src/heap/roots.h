#ifndef _ROOTS
#define _ROOTS

#include "heap/values.h"
#include "heap/heap.h"
#include "io/ast.h"
#include "utils/globals.h"

#define FOR_EACH_SIMPLE_ROOT(VISIT)                                  \
  VISIT(Type, string_type, new_type(STRING_TAG))                     \
  VISIT(Type, tuple_type, new_type(TUPLE_TAG))                       \
  VISIT(Type, void_type, new_type(VOID_TAG))                         \
  VISIT(Void, vhoid, new_singleton(void_type()))                     \
  VISIT(Type, null_type, new_type(NULL_TAG))                         \
  VISIT(Null, nuhll, new_singleton(null_type()))                     \
  VISIT(Type, true_type, new_type(TRUE_TAG))                         \
  VISIT(True, thrue, new_singleton(true_type()))                     \
  VISIT(Type, false_type, new_type(FALSE_TAG))                       \
  VISIT(False, fahlse, new_singleton(false_type()))                  \
  VISIT(Type, literal_type, new_type(LITERAL_TAG))                   \
  VISIT(Type, dictionary_type, new_type(DICTIONARY_TAG))             \
  VISIT(Type, lambda_type, new_type(LAMBDA_TAG))                     \
  VISIT(Dictionary, toplevel, new_dictionary())                      \
  VISIT(Type, buffer_type, new_type(BUFFER_TAG))                     \
  VISIT(Type, code_type, new_type(CODE_TAG))

#define FOR_EACH_COMPLICATED_ROOT(VISIT)                             \
  VISIT(Type, type_type, NULL)

#define FOR_EACH_ROOT(VISIT)                                         \
  FOR_EACH_COMPLICATED_ROOT(VISIT)                                   \
  FOR_EACH_SIMPLE_ROOT(VISIT)

namespace neutrino {

class RootContainer {
public:
  RootContainer();
  
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

#endif // _ROOTS
