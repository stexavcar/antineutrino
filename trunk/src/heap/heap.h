#ifndef _HEAP
#define _HEAP

#include "heap/space.h"
#include "heap/values.h"
#include "utils/string.h"

namespace neutrino {

class Heap {
public:
  Heap(RootContainer &roots);

  /**
   * Allocates a new object of the specified size and returns it
   * without initializing its fields.  IMPORTANT NOTE: all fields
   * of the returned object that are expected by the garbage collector
   * to contain object pointers must have been initialized before any
   * more memory is allocated.
   */
  Data *allocate_object(uint32_t size, Type *type);
  
  /**
   * Creates and initializes a new type object but does not check that
   * the object is valid.  This function should only be used directly
   * during bootstrapping.
   */
  Data *allocate_type(TypeTag instance_type);
  
  /**
   * Creates and validates a new type object.
   */
  Data *new_type(TypeTag instance_type);
  
  Data *new_string(string value);

  Data *new_tuple(uint32_t length);
  
  Data *new_singleton(Type *type);
  
  Data *new_dictionary();
  
  Data *new_lambda(uint32_t argc, Code *code, Tuple *literals);
  
  Data *new_literal(Value *value);
  
  Data *new_abstract_buffer(uint32_t size, Type *type);
  
  template <typename T> Data *new_buffer(uint32_t size);
  
  Data *new_code(uint32_t size);

  Space &space() { return space_; }
  RootContainer &roots() { return roots_; }
private:
  RootContainer &roots_;
  Space space_;
};

}

#endif // _HEAP
