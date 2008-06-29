#ifndef _HEAP
#define _HEAP

#include "compiler/ast.h"
#include "heap/memory.h"
#include "runtime/context.h"
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
  Allocation<Object> allocate_object(uword size, Layout *layout);

  template <class C>
  Allocation<C> allocate_object(uword size, Layout *layout);

  /**
   * Creates and initializes a new type object but does not check that
   * the object is valid.  This function should only be used directly
   * during bootstrapping.
   */
  Allocation<Layout> allocate_layout(InstanceType instance_type);

  Allocation<Layout> allocate_empty_layout(InstanceType instance_type);

  Allocation<Protocol> allocate_empty_protocol();

  Allocation<Layout> new_layout(InstanceType instance_type,
      uword instance_field_count, Immediate *protocol, Tuple *methods);

  Allocation<Protocol> new_protocol(Tuple *methods, Value *super,
      Immediate *name);

  Allocation<Context> new_context();

  Option<Task> new_task(Architecture &arch);

  Allocation<Stack> new_stack(uword height);

  Allocation<String> new_string(string value);

  Allocation<String> new_string(uword length);

  Allocation<Tuple> new_tuple(uword length);

  Allocation<Array> new_array(uword length);

  Allocation<Singleton> new_singleton(Layout *layout);
  template <class C> Allocation<C> new_singleton(Layout *layout);

  Allocation<Symbol> new_symbol(Value *name);

  Allocation<LocalVariable> new_local_variable(Symbol *symbol);

  Allocation<HashMap> new_hash_map();

  Allocation<HashMap> new_hash_map(Tuple *store);

  Allocation<Lambda> new_lambda(uword argc, uword max_stack_height,
      Value *code, Value *literals, Value *tree, Context *context);

  Allocation<Cell> new_cell(Value *value);

  Allocation<Parameters> new_parameters(Smi *position_count, Tuple *params);

  Allocation<QuoteTemplate> new_quote_template(SyntaxTree *body, Tuple *unquotes);

  Allocation<LambdaExpression> new_lambda_expression(Parameters *params, SyntaxTree *body,
      bool is_local);

  Allocation<ReturnExpression> new_return_expression(SyntaxTree *value);

  Allocation<LiteralExpression> new_literal_expression(Value *value);

  Allocation<Lambda> allocate_lambda(uword argc);

  Allocation<Channel> allocate_channel();

#define DECLARE_ALLOCATOR(n, Name, name)                             \
  Allocation<Name> allocate_##name();
eBoilerplateAllocator(DECLARE_ALLOCATOR)
#undef MAKE_ALLOCATOR

  Allocation<AbstractBuffer> new_abstract_buffer(uword size, Layout *layout);

  template <typename T> Allocation<Buffer> new_buffer(uword size);

  Allocation<Buffer> new_buffer(uword size);

  Allocation<Code> new_code(uword size);

  Allocation<Method> new_method(Selector *selector, Signature *signature,
      Lambda *lambda);

  Allocation<Signature> new_signature(Tuple *parameters);

  Allocation<Instance> new_instance(Layout *layout);

  Allocation<Selector> new_selector(Immediate *name, Smi *argc, Bool *is_accessor);

  Allocation<Forwarder> new_forwarder(Forwarder::Type type, Value *target);
  Allocation<ForwarderDescriptor> new_forwarder_descriptor(Forwarder::Type type, Value *target);

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
