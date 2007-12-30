#ifndef _HEAP_FACTORY
#define _HEAP_FACTORY

#include "heap/values.h"
#include "utils/string.h"
#include "utils/types.h"

namespace neutrino {

class Factory {
public:
  Factory(Runtime &heap);
  ref<Layout> allocate_empty_class(InstanceType instance_type);
  ref<String> new_string(string str);
  ref<String> new_string(uint32_t length);
  ref<Tuple> new_tuple(uint32_t size);
  ref<Lambda> new_lambda(uint32_t argc, ref<Value> code,
      ref<Value> literals, ref<Value> tree);
  ref<LambdaExpression> new_lambda_expression(ref<Tuple> params,
      ref<SyntaxTree> body);
  ref<ReturnExpression> new_return_expression(ref<SyntaxTree> value);
  ref<QuoteTemplate> new_quote_template(ref<SyntaxTree> body, ref<Tuple> unquotes);
  ref<LiteralExpression> new_literal_expression(ref<Value> value);
  ref<Method> new_method(ref<String> name, ref<Lambda> lambda);
  ref<Symbol> new_symbol(ref<Value> value);
  ref<Dictionary> new_dictionary();
  ref<Code> new_code(uint32_t size);
  ref<Layout> new_class(InstanceType instance_type, 
      uint32_t instance_field_count, ref<Tuple> methods,
      ref<Value> super, ref<Value> name);
  ref<Instance> new_instance(ref<Layout> chlass);
private:
  template <typename T, class M> inline ref<T> allocate(M fun);
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _HEAP_FACTORY
