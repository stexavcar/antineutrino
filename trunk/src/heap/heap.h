#ifndef _HEAP
#define _HEAP

#include "heap/memory.h"
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
   * to contain object pointers must have been initialized before a
   * gc can be run.
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
  
  Data *new_lambda(uint32_t argc, Value *code, Value *literals, LambdaExpression *tree);
  
  Data *new_builtin_call(uint32_t argc, uint32_t index);
  
  Data *allocate_lambda(uint32_t argc);

  Data *allocate_literal_expression();
  
  Data *allocate_invoke_expression();
  
  Data *allocate_call_expression();
  
  Data *allocate_conditional_expression();

  Data *allocate_class_expression();
  
  Data *allocate_return_expression();
  
  Data *allocate_method_expression();
  
  Data *allocate_sequence_expression();
  
  Data *allocate_tuple_expression();
  
  Data *allocate_global_expression();
  
  Data *new_this_expression();

  Data *allocate_quote_expression();

  Data *allocate_lambda_expression();
  
  Data *allocate_interpolate_expression();

  Data *allocate_symbol();
  
  Data *allocate_method();
  
  Data *new_abstract_buffer(uint32_t size, Class *chlass);
  
  template <typename T> Data *new_buffer(uint32_t size);
  
  Data *new_code(uint32_t size);
  
  Data *new_method(String *name, Lambda *lambda);
    
  Data *new_instance(Class *chlass);

  Memory &memory() { return memory_; }
  Roots &roots() { return roots_; }
private:
  Roots &roots_;
  Memory memory_;
};

}

#endif // _HEAP
