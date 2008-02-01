#ifndef _HEAP
#define _HEAP

#include "heap/memory.h"
#include "runtime/context.h"
#include "utils/string.h"
#include "values/values.h"

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
  Data *allocate_object(uint32_t size, Layout *layout);
  
  /**
   * Creates and initializes a new type object but does not check that
   * the object is valid.  This function should only be used directly
   * during bootstrapping.
   */
  Data *allocate_layout(InstanceType instance_type);
  
  Data *allocate_empty_layout(InstanceType instance_type);
  
  Data *allocate_empty_protocol();

  Data *new_layout(InstanceType instance_type, uint32_t instance_field_count,
      Tuple *methods, Value *super, Value *name);
  
  Data *new_protocol(Tuple *methods, Value *super, Value *name);
  
  Data *new_context();
  
  Data *new_task();
  
  Data *new_stack(uint32_t height);
  
  Data *new_string(string value);

  Data *new_string(uint32_t length);

  Data *new_tuple(uint32_t length);
  
  Data *new_singleton(Layout *layout);
  
  Data *new_symbol(Value *name);
  
  Data *new_dictionary();
  
  Data *new_dictionary(Tuple *store);
  
  Data *new_lambda(uint32_t argc, Value *code, Value *literals, Value *tree,
      Context *context);
  
  Data *new_quote_template(SyntaxTree *body, Tuple *unquotes);
  
  Data *new_lambda_expression(Tuple *params, SyntaxTree *body);
  
  Data *new_return_expression(SyntaxTree *value);
  
  Data *new_literal_expression(Value *value);
  
  Data *allocate_lambda(uint32_t argc);
  
  Data *allocate_builtin_call();
  
  Data *allocate_unquote_expression();
  
#define DECLARE_ALLOCATOR(n, NAME, Name, name)                       \
  Data *allocate_##name();
FOR_EACH_GENERATABLE_TYPE(DECLARE_ALLOCATOR)
#undef MAKE_ALLOCATOR
  
  Data *new_abstract_buffer(uint32_t size, Layout *layout);
  
  template <typename T> Data *new_buffer(uint32_t size);

  Data *new_code(uint32_t size);

  Data *new_method(String *name, Signature *signature, Lambda *lambda);
  
  Data *new_signature(Tuple *parameters);

  Data *new_instance(Layout *layout);

  Memory &memory() { return memory_; }
  Roots &roots() { return roots_; }
private:
  Roots &roots_;
  Memory memory_;
  
  static Watch<Counter> &allocation_count() { return allocation_count_; }
  static Watch<Counter> allocation_count_;
};

}

#endif // _HEAP
