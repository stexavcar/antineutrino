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
  allocation<Object> allocate_object(uword size, Layout *layout);

  template <class C>
  allocation<C> allocate_object(uword size, Layout *layout);

  /**
   * Creates and initializes a new type object but does not check that
   * the object is valid.  This function should only be used directly
   * during bootstrapping.
   */
  allocation<Layout> allocate_layout(InstanceType instance_type);

  allocation<Layout> allocate_empty_layout(InstanceType instance_type);

  allocation<Protocol> allocate_empty_protocol();

  allocation<Layout> new_layout(InstanceType instance_type,
      uword instance_field_count, Immediate *protocol, Tuple *methods);

  allocation<Protocol> new_protocol(Tuple *methods, Value *super,
      Immediate *name);

  allocation<Context> new_context();

  maybe<Task> new_task(Architecture &arch);

  allocation<Stack> new_stack(uword height);

  allocation<String> new_string(string value);

  allocation<String> new_string(uword length);

  allocation<Tuple> new_tuple(uword length);

  allocation<Array> new_array(uword length);

  allocation<Singleton> new_singleton(Layout *layout);
  template <class C> allocation<C> new_singleton(Layout *layout);

  allocation<Symbol> new_symbol(Value *name);

  allocation<LocalVariable> new_local_variable(Symbol *symbol);

  allocation<HashMap> new_hash_map();

  allocation<HashMap> new_hash_map(Tuple *store);

  allocation<Lambda> new_lambda(uword argc, uword max_stack_height,
      Value *code, Value *literals, Value *tree, Context *context);

  allocation<Cell> new_cell(Value *value);

  allocation<Parameters> new_parameters(Smi *position_count, Tuple *params);

  allocation<QuoteTemplate> new_quote_template(SyntaxTree *body, Tuple *unquotes);

  allocation<LambdaExpression> new_lambda_expression(Parameters *params, SyntaxTree *body,
      bool is_local);

  allocation<ReturnExpression> new_return_expression(SyntaxTree *value);

  allocation<LiteralExpression> new_literal_expression(Value *value);

  allocation<Lambda> allocate_lambda(uword argc);

  allocation<Channel> allocate_channel();

#define DECLARE_ALLOCATOR(n, Name, name)                             \
  allocation<Name> allocate_##name();
eBoilerplateAllocator(DECLARE_ALLOCATOR)
#undef MAKE_ALLOCATOR

  allocation<AbstractBuffer> new_abstract_buffer(uword size, Layout *layout);

  template <typename T> allocation<Buffer> new_buffer(uword size);

  allocation<Buffer> new_buffer(uword size);

  allocation<Code> new_code(uword size);

  allocation<Method> new_method(Selector *selector, Signature *signature,
      Lambda *lambda);

  allocation<Signature> new_signature(Tuple *parameters);

  allocation<Instance> new_instance(Layout *layout);

  allocation<Selector> new_selector(Immediate *name, Smi *argc, Bool *is_accessor);

  allocation<Forwarder> new_forwarder(Forwarder::Type type, Value *target);
  allocation<ForwarderDescriptor> new_forwarder_descriptor(Forwarder::Type type, Value *target);

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
