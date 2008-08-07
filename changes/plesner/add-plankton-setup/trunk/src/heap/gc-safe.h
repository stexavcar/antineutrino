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
  probably<Layout> allocate_empty_layout(InstanceType instance_type);
  probably<String> new_string(string str);
  probably<String> new_string(uword length);
  probably<Tuple> new_tuple(uword size);
  probably<Parameters> new_parameters(ref<Smi> position_count, ref<Tuple> params);
  probably<Lambda> new_lambda(uword argc, uword max_stack_height, ref<Value> code,
      ref<Value> literals, ref<Value> tree, ref<Context> context);
  probably<Task> new_task(Architecture &arch);
  probably<LambdaExpression> new_lambda_expression(ref<Parameters> params,
      ref<SyntaxTree> body, bool is_local);
  probably<ReturnExpression> new_return_expression(ref<SyntaxTree> value);
  probably<QuoteTemplate> new_quote_template(ref<SyntaxTree> body, ref<Tuple> unquotes);
  probably<LiteralExpression> new_literal_expression(ref<Value> value);
  probably<Signature> new_signature(ref<Tuple> parameters);
  probably<Method> new_method(ref<Selector> selector, ref<Signature> signature,
      ref<Lambda> lambda);
  probably<Symbol> new_symbol(ref<Value> value);
  probably<HashMap> new_hash_map();
  probably<Code> new_code(uword size);
  probably<Layout> new_layout(InstanceType instance_type,
      uword instance_field_count, ref<Immediate> protocol,
      ref<Tuple> methods);
  probably<Protocol> new_protocol(ref<Tuple> methods, ref<Value> super,
      ref<Immediate> name);
  probably<Instance> new_instance(ref<Layout> layout);
  probably<Selector> new_selector(ref<Immediate> name, Smi *argc, ref<Bool> is_accessor);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _HEAP_FACTORY
