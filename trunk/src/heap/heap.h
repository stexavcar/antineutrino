#ifndef _HEAP
#define _HEAP

#include "heap/space.h"
#include "heap/values.h"
#include "utils/string.h"

namespace neutrino {

class Heap {
public:
  Heap(Roots &roots);

  /**
   * Allocates a new object of the specified size and returns it
   * without initializing its fields.  IMPORTANT NOTE: all fields
   * of the returned object that are expected by the garbage collector
   * to contain object pointers must have been initialized before any
   * more memory is allocated.
   */
  Data *allocate_object(uint32_t size, Class *chlass);
  
  /**
   * Creates and initializes a new type object but does not check that
   * the object is valid.  This function should only be used directly
   * during bootstrapping.
   */
  Data *allocate_class(InstanceType instance_type);
  
  /**
   * Creates and validates a new type object.
   */
  Data *new_empty_class(InstanceType instance_type);
  
  Data *new_string(string value);

  Data *new_string(uint32_t length);

  Data *new_tuple(uint32_t length);
  
  Data *new_singleton(Class *chlass);
  
  Data *new_dictionary();
  
  Data *new_dictionary(Tuple *store);
  
  Data *new_lambda(uint32_t argc, Code *code, Tuple *literals);
  
  Data *new_lambda(uint32_t argc);

  Data *new_literal_expression();
  
  Data *new_invoke_expression();
  
  Data *new_class_expression();
  
  Data *new_return_expression();
  
  Data *new_method_expression();
  
  Data *new_abstract_buffer(uint32_t size, Class *chlass);
  
  template <typename T> Data *new_buffer(uint32_t size);
  
  Data *new_code(uint32_t size);
  
  Data *new_method(String *name, Lambda *lambda);
  
  Data *new_method();
  
  Data *new_instance(Class *chlass);

  Space &space() { return space_; }
  Roots &roots() { return roots_; }
private:
  Roots &roots_;
  Space space_;
};

}

#endif // _HEAP
