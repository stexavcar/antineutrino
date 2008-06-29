#ifndef _HEAP_FACTORY
#define _HEAP_FACTORY

#include "utils/string.h"
#include "utils/types.h"
#include "values/values.h"

namespace neutrino {

class GcSafe {
public:
  GcSafe(Runtime &runtime);
  void set(ref<HashMap> dict, ref<Value> key, ref<Value> value);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

class Factory {
public:
  Factory(Runtime &heap);
  Option<Layout> allocate_empty_layout(InstanceType instance_type);
  Option<String> new_string(string str);
  Option<String> new_string(uword length);
  Option<Tuple> new_tuple(uword size);
  Option<Parameters> new_parameters(ref<Smi> position_count, ref<Tuple> params);
  Option<Lambda> new_lambda(uword argc, uword max_stack_height, ref<Value> code,
      ref<Value> literals, ref<Value> tree, ref<Context> context);
  Option<Task> new_task(Architecture &arch);
  Option<LambdaExpression> new_lambda_expression(ref<Parameters> params,
      ref<SyntaxTree> body, bool is_local);
  Option<ReturnExpression> new_return_expression(ref<SyntaxTree> value);
  Option<QuoteTemplate> new_quote_template(ref<SyntaxTree> body, ref<Tuple> unquotes);
  Option<LiteralExpression> new_literal_expression(ref<Value> value);
  Option<Signature> new_signature(ref<Tuple> parameters);
  Option<Method> new_method(ref<Selector> selector, ref<Signature> signature,
      ref<Lambda> lambda);
  Option<Symbol> new_symbol(ref<Value> value);
  Option<HashMap> new_hash_map();
  Option<Code> new_code(uword size);
  Option<Layout> new_layout(InstanceType instance_type,
      uword instance_field_count, ref<Immediate> protocol,
      ref<Tuple> methods);
  Option<Protocol> new_protocol(ref<Tuple> methods, ref<Value> super,
      ref<Immediate> name);
  Option<Instance> new_instance(ref<Layout> layout);
  Option<Selector> new_selector(ref<Immediate> name, Smi *argc, ref<Bool> is_accessor);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _HEAP_FACTORY
