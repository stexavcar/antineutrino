#include "compiler/ast-inl.h"
#include "heap/heap-inl.h"
#include "heap/memory-inl.h"
#include "heap/pointer.h"
#include "heap/roots.h"
#include "heap/values-inl.h"
#include "utils/string-inl.h"
#include "utils/types-inl.h"

using namespace neutrino;

Heap::Heap(Roots &roots)
    : roots_(roots)
    , memory_(*this) { }

Data *Heap::allocate_object(uint32_t size, Layout *type) {
  address addr = memory().allocate(size);
  if (!addr) return AllocationFailed::make(size);
  Object *result = ValuePointer::tag_as_object(addr);
  result->set_layout(type);
#ifdef DEBUG
  uint32_t *fields = reinterpret_cast<uint32_t*>(addr);
  for (uint32_t i = 1; i < size / kPointerSize; i++)
    fields[i] = ValuePointer::kUninitialized;
#endif
  return result;
}

Data *Heap::allocate_layout(InstanceType instance_type) {
  Data *val = allocate_object(Layout::kSize, roots().layout_layout());
  if (is<AllocationFailed>(val)) return val;
  Layout *result = reinterpret_cast<Layout*>(cast<Object>(val));
  result->set_layout(roots().layout_layout());
  result->set_instance_type(instance_type);
  result->set_super(Smi::from_int(0));
  return result;
}

Data *Heap::new_lambda(uint32_t argc, Value *code, Value *constant_pool,
    Value *tree, Context *context) {
  Data *val = allocate_object(Lambda::kSize, roots().lambda_layout());
  if (is<AllocationFailed>(val)) return val;
  Lambda *result = cast<Lambda>(val);
  result->set_argc(argc);
  result->set_code(code);
  result->set_constant_pool(constant_pool);
  result->set_tree(tree);
  result->set_outers(roots().empty_tuple());
  result->set_context(context);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_quote_template(SyntaxTree *value, Tuple *unquotes) {
  Data *val = allocate_object(QuoteTemplate::kSize, roots().quote_template_layout());
  if (is<AllocationFailed>(val)) return val;
  QuoteTemplate *result = cast<QuoteTemplate>(val);
  result->set_value(value);
  result->set_unquotes(unquotes);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_lambda_expression(Tuple *params, SyntaxTree *body) {
  Data *val = allocate_object(LambdaExpression::kSize, roots().lambda_expression_layout());
  if (is<AllocationFailed>(val)) return val;
  LambdaExpression *result = cast<LambdaExpression>(val);
  result->set_params(params);
  result->set_body(body);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_literal_expression(Value *value) {
  Data *val = allocate_object(LiteralExpression::kSize, roots().literal_expression_layout());
  if (is<AllocationFailed>(val)) return val;
  LiteralExpression *result = cast<LiteralExpression>(val);
  result->set_value(value);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_return_expression(SyntaxTree *value) {
  Data *val = allocate_object(ReturnExpression::kSize, roots().return_expression_layout());
  if (is<AllocationFailed>(val)) return val;
  ReturnExpression *result = cast<ReturnExpression>(val);
  result->set_value(value);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::allocate_lambda(uint32_t argc) {
  Data *val = allocate_object(Lambda::kSize, roots().lambda_layout());
  if (is<AllocationFailed>(val)) return val;
  Lambda *result = cast<Lambda>(val);
  result->set_argc(argc);
  return result;
}

Data *Heap::allocate_builtin_call() {
  return allocate_object(BuiltinCall::kSize, roots().builtin_call_layout());
}

Data *Heap::allocate_unquote_expression() {
  return allocate_object(UnquoteExpression::kSize, roots().unquote_expression_layout());
}

#define MAKE_ALLOCATOR(n, NAME, Name, name)                          \
Data *Heap::allocate_##name() {                                      \
  return allocate_object(Name::kSize, roots().name##_layout());       \
}
FOR_EACH_GENERATABLE_TYPE(MAKE_ALLOCATOR)
#undef MAKE_ALLOCATOR

Data *Heap::allocate_empty_layout(InstanceType instance_type) {
  Data *val = allocate_layout(instance_type);
  if (is<AllocationFailed>(val)) return val;
  ASSERT_IS(Layout, val);
  Layout *result = cast<Layout>(val);
  result->set_instance_field_count(0);
  result->set_super(Smi::from_int(0));
  result->set_name(Smi::from_int(0));
  result->set_methods(roots().empty_tuple());
  ASSERT(result->is_empty());
  return result;
}

Data *Heap::new_context() {
  Data *val = allocate_object(Context::kSize, roots().context_layout());
  if (is<AllocationFailed>(val)) return val;
  Context *result = cast<Context>(val);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_layout(InstanceType instance_type, 
    uint32_t instance_field_count, Tuple *methods, Value *super,
    Value *name) {
  Data *val = allocate_layout(instance_type);
  if (is<AllocationFailed>(val)) return val;
  ASSERT_IS(Layout, val);
  Layout *result = cast<Layout>(val);
  result->set_instance_field_count(instance_field_count);
  result->set_super(super);
  result->set_methods(methods);
  result->set_name(name);
  IF_PARANOID(result->validate());
  return result;  
}

Data *Heap::allocate_empty_protocol() {
  return allocate_object(Protocol::kSize, roots().protocol_layout());
}

Data *Heap::new_method(String *name, Lambda *lambda) {
  Data *val = allocate_object(Method::kSize, roots().method_layout());
  if (is<AllocationFailed>(val)) return val;
  Method *result = cast<Method>(val);
  result->set_name(name);
  result->set_lambda(lambda);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_string(string value) {
  uint32_t size = String::size_for(value.length());
  Data *val = allocate_object(size, roots().string_layout());
  if (is<AllocationFailed>(val)) return val;
  String *result = cast<String>(val);
  result->set_length(value.length());
  for (uint32_t i = 0; i < value.length(); i++)
    result->set(i, value[i]);
  ASSERT_EQ(size, result->size_in_memory());
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_stack(uint32_t height) {
  uint32_t size = Stack::size_for(height);
  Data *val = allocate_object(size, roots().stack_layout());
  if (is<AllocationFailed>(val)) return val;
  Stack *result = cast<Stack>(val);
  result->set_height(height);
  result->set_fp(0);
  result->set_top_marker(0);
  result->set_status(Stack::Status());
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_task() {
  Data *stack_val = new_stack(Stack::kInitialHeight);
  if (is<AllocationFailed>(stack_val)) return stack_val;
  Data *task_val = allocate_object(Task::kSize, roots().task_layout());
  if (is<AllocationFailed>(task_val)) return task_val;
  Task *result = cast<Task>(task_val);
  result->set_stack(cast<Stack>(stack_val));
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_string(uint32_t length) {
  uint32_t size = String::size_for(length);
  Data *val = allocate_object(size, roots().string_layout());
  if (is<AllocationFailed>(val)) return val;
  String *result = cast<String>(val);
  result->set_length(length);
  ASSERT_EQ(size, result->size_in_memory());
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_code(uint32_t size) {
  Data *val = new_abstract_buffer(sizeof(uint16_t) * size, roots().code_layout());
  if (is<AllocationFailed>(val)) return val;
  Code *result = cast<Code>(val);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_abstract_buffer(uint32_t byte_count, Layout *type) {
  uint32_t size = AbstractBuffer::size_for(byte_count);
  Data *val = allocate_object(size, type);
  if (is<AllocationFailed>(val)) return val;
  AbstractBuffer *result = cast<AbstractBuffer>(val);
  result->set_size<uint8_t>(byte_count);
  ASSERT_EQ(size, result->size_in_memory());
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_tuple(uint32_t length) {
  uint32_t size = Tuple::size_for(length);
  Data *val = allocate_object(size, roots().tuple_layout());
  if (is<AllocationFailed>(val)) return val;
  Tuple *result = cast<Tuple>(val);
  result->set_length(length);
  for (uint32_t i = 0; i < length; i++)
    result->set(i, roots().vhoid());
  ASSERT_EQ(size, result->size_in_memory());
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_symbol(Value *name) {
  Data *val = allocate_object(Symbol::kSize, roots().symbol_layout());
  if (is<AllocationFailed>(val)) return val;
  Symbol *result = cast<Symbol>(val);
  result->set_name(name);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_singleton(Layout *type) {
  return allocate_object(Singleton::kSize, type);
}

Data *Heap::new_dictionary() {
  Data *table_val = new_tuple(0);
  if (is<AllocationFailed>(table_val)) return table_val;
  return new_dictionary(cast<Tuple>(table_val));
}

Data *Heap::new_dictionary(Tuple *table) {
  Data *val = allocate_object(Dictionary::kSize, roots().dictionary_layout());
  if (is<AllocationFailed>(val)) return val;
  Dictionary *result = cast<Dictionary>(val);
  result->set_table(table);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_instance(Layout *layout) {
  ASSERT_EQ(INSTANCE_TYPE, layout->instance_type());
  uint32_t field_count = layout->instance_field_count();
  uint32_t size = Instance::size_for(field_count);
  Data *val = allocate_object(size, layout);
  if (is<AllocationFailed>(val)) return val;
  Instance *result = cast<Instance>(val);
  for (uint32_t i = 0; i < field_count; i++)
    result->set_field(i, roots().vhoid());
  IF_PARANOID(result->validate());
  return result;
}
