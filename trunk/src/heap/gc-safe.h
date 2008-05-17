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
  ref<Layout> allocate_empty_layout(InstanceType instance_type);
  ref<String> new_string(string str);
  ref<String> new_string(uword length);
  ref<Tuple> new_tuple(uword size);
  ref<Parameters> new_parameters(ref<Smi> position_count, ref<Tuple> params);
  ref<Lambda> new_lambda(uword argc, ref<Value> code,
      ref<Value> literals, ref<Value> tree, ref<Context> context);
  ref<Task> new_task();
  ref<LambdaExpression> new_lambda_expression(ref<Parameters> params,
      ref<SyntaxTree> body, bool is_local);
  ref<ReturnExpression> new_return_expression(ref<SyntaxTree> value);
  ref<QuoteTemplate> new_quote_template(ref<SyntaxTree> body, ref<Tuple> unquotes);
  ref<LiteralExpression> new_literal_expression(ref<Value> value);
  ref<Signature> new_signature(ref<Tuple> parameters);
  ref<Method> new_method(ref<Selector> selector, ref<Signature> signature,
      ref<Lambda> lambda);
  ref<Symbol> new_symbol(ref<Value> value);
  ref<HashMap> new_hash_map();
  ref<Code> new_code(uword size);
  ref<Layout> new_layout(InstanceType instance_type, 
      uword instance_field_count, ref<Immediate> protocol,
      ref<Tuple> methods);
  ref<Protocol> new_protocol(ref<Tuple> methods, ref<Value> super,
      ref<Immediate> name);
  ref<Instance> new_instance(ref<Layout> layout);
  ref<Selector> new_selector(ref<Immediate> name, Smi *argc, ref<Bool> is_accessor);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _HEAP_FACTORY
