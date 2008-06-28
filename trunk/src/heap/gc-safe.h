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
  Data *allocate_empty_layout(InstanceType instance_type);
  Data *new_string(string str);
  Data *new_string(uword length);
  Data *new_tuple(uword size);
  Data *new_parameters(ref<Smi> position_count, ref<Tuple> params);
  Data *new_lambda(uword argc, uword max_stack_height, ref<Value> code,
      ref<Value> literals, ref<Value> tree, ref<Context> context);
  Data *new_task(Architecture &arch);
  Data *new_lambda_expression(ref<Parameters> params,
      ref<SyntaxTree> body, bool is_local);
  Data *new_return_expression(ref<SyntaxTree> value);
  Data *new_quote_template(ref<SyntaxTree> body, ref<Tuple> unquotes);
  Data *new_literal_expression(ref<Value> value);
  Data *new_signature(ref<Tuple> parameters);
  Data *new_method(ref<Selector> selector, ref<Signature> signature,
      ref<Lambda> lambda);
  Data *new_symbol(ref<Value> value);
  Data *new_hash_map();
  Data *new_code(uword size);
  Data *new_layout(InstanceType instance_type,
      uword instance_field_count, ref<Immediate> protocol,
      ref<Tuple> methods);
  Data *new_protocol(ref<Tuple> methods, ref<Value> super,
      ref<Immediate> name);
  Data *new_instance(ref<Layout> layout);
  Data *new_selector(ref<Immediate> name, Smi *argc, ref<Bool> is_accessor);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _HEAP_FACTORY
