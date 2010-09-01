#include "compiler/ast-inl.h"
#include "heap/heap-inl.pp.h"
#include "heap/memory-inl.h"
#include "heap/pointer.h"
#include "heap/roots.h"
#include "monitor/monitor-inl.h"
#include "utils/string-inl.pp.h"
#include "utils/types-inl.h"
#include "values/values-inl.pp.h"

namespace neutrino {

Watch<Counter> Heap::allocation_count_("allocation count");

Heap::Heap(Roots &roots)
    : roots_(roots)
    , memory_(*this) { }

allocation<Object> Heap::allocate_object(uword size, Layout *type) {
  allocation_count()->increment();
  address addr = memory().allocate(size);
  if (!addr) return AllocationFailed::make(size);
  Object *result = ValuePointer::tag_as_object(addr);
  result->set_layout(type);
#ifdef DEBUG
  uword *fields = reinterpret_cast<uword*>(addr);
  for (uword i = 1; i < size / kPointerSize; i++)
    fields[i] = ValuePointer::kUninitialized;
#endif
  return result;
}

allocation<SimpleLayout> Heap::allocate_simple_layout(InstanceType instance_type) {
  @assert instance_type != tInstance;
  allocation<Object> val = allocate_object(SimpleLayout::kSize, roots().layout_layout());
  if (val.has_failed()) return allocation<SimpleLayout>(val.signal());
  SimpleLayout *result = reinterpret_cast<SimpleLayout*>(cast<Object>(val.data()));
  result->set_layout(roots().layout_layout());
  result->set_instance_type(instance_type);
  result->set_protocol(Smi::from_int(0));
  return result;
}

allocation<InstanceLayout> Heap::allocate_instance_layout() {
  allocation<Object> val = allocate_object(InstanceLayout::kSize, roots().layout_layout());
  if (val.has_failed()) return allocation<InstanceLayout>(val.signal());
  InstanceLayout *result = reinterpret_cast<InstanceLayout*>(cast<Object>(val.data()));
  result->set_layout(roots().layout_layout());
  result->set_instance_type(tInstance);
  result->set_protocol(Smi::from_int(0));
  return result;
}

allocation<Lambda> Heap::new_lambda(uword argc, uword max_stack_height,
    Value *code, Value *constant_pool, Value *tree, Context *context) {
  @alloc Lambda *result = allocate_object<Lambda>(Lambda::kSize, roots().lambda_layout());
  result->set_argc(argc);
  result->set_max_stack_height(max_stack_height);
  result->set_code(code);
  result->set_constant_pool(constant_pool);
  result->set_tree(tree);
  result->set_outers(roots().empty_tuple());
  result->set_context(context);
  IF_PARANOID(result->validate());
  return result;
}

allocation<Cell> Heap::new_cell(Value *value) {
  @alloc Cell *result = allocate_object<Cell>(Cell::kSize, roots().cell_layout());
  result->set_value(value);
  IF_PARANOID(result->validate());
  return result;
}

allocation<Parameters> Heap::new_parameters(Smi *position_count, Tuple *params) {
  @alloc Parameters *result = allocate_object<Parameters>(Parameters::kSize, roots().parameters_layout());
  result->set_position_count(position_count);
  result->set_parameters(params);
  IF_PARANOID(result->validate());
  return result;
}

allocation<LambdaExpression> Heap::new_lambda_expression(
    Parameters *params, SyntaxTree *body, bool is_local) {
  @alloc LambdaExpression *result = allocate_object<LambdaExpression>(LambdaExpression::kSize, roots().lambda_expression_layout());
  result->set_parameters(params);
  result->set_body(body);
  result->set_is_local(is_local ? static_cast<Bool*>(roots().thrue()) : static_cast<Bool*>(roots().fahlse()));
  IF_PARANOID(result->validate());
  return result;
}

allocation<LiteralExpression> Heap::new_literal_expression(Value *value) {
  @alloc LiteralExpression *result = allocate_object<LiteralExpression>(LiteralExpression::kSize, roots().literal_expression_layout());
  result->set_value(value);
  IF_PARANOID(result->validate());
  return result;
}

allocation<ReturnExpression> Heap::new_return_expression(SyntaxTree *value) {
  @alloc ReturnExpression *result = allocate_object<ReturnExpression>(ReturnExpression::kSize, roots().return_expression_layout());
  result->set_value(value);
  IF_PARANOID(result->validate());
  return result;
}

allocation<Lambda> Heap::allocate_lambda(uword argc) {
  @alloc Lambda *result = allocate_object<Lambda>(Lambda::kSize, roots().lambda_layout());
  result->set_argc(argc);
  return result;
}

allocation<Channel> Heap::allocate_channel() {
  @alloc Channel *result = allocate_object<Channel>(Channel::kSize, roots().channel_layout());
  result->set_is_connected(roots().fahlse());
  result->set_proxy(NULL);
  return result;
}


template <class C>
allocation<C> Heap::allocate_object(uword size, Layout *layout) {
  @alloc(Object) C *result = allocate_object(size, layout);
  return result;
}


#define MAKE_ALLOCATOR(n, Name, name)                                \
allocation<Name> Heap::allocate_##name() {                     \
  return allocate_object<Name>(Name::kSize, roots().name##_layout());\
}
eBoilerplateAllocator(MAKE_ALLOCATOR)
#undef MAKE_ALLOCATOR


allocation<InstanceLayout> Heap::allocate_empty_instance_layout() {
  @alloc InstanceLayout *result = allocate_instance_layout();
  result->set_instance_field_count(0);
  result->set_protocol(Smi::from_int(0));
  ASSERT(result->is_empty());
  return result;
}


allocation<InstanceLayout> Heap::new_instance_layout(uword instance_field_count,
    Immediate *protocol, Tuple *methods) {
  @alloc InstanceLayout *result = allocate_instance_layout();
  result->set_instance_field_count(instance_field_count);
  result->set_protocol(protocol);
  result->set_methods(methods);
  IF_PARANOID(result->validate());
  return result;
}


allocation<SimpleLayout> Heap::allocate_empty_simple_layout(InstanceType instance_type) {
  @alloc SimpleLayout *result = allocate_simple_layout(instance_type);
  result->set_protocol(Smi::from_int(0));
  ASSERT(result->is_empty());
  return cast<SimpleLayout>(result);
}


allocation<SimpleLayout> Heap::new_simple_layout(InstanceType instance_type,
    Immediate *protocol, Tuple *methods) {
  @alloc SimpleLayout *result = allocate_simple_layout(instance_type);
  result->set_protocol(protocol);
  result->set_methods(methods);
  IF_PARANOID(result->validate());
  return result;
}



allocation<Context> Heap::new_context() {
  @alloc Context *result = allocate_object<Context>(Context::kSize, roots().context_layout());
  IF_PARANOID(result->validate());
  return result;
}


allocation<Protocol> Heap::new_protocol(Tuple *methods, Value *super, Immediate *name) {
  @alloc Protocol *result = allocate_object<Protocol>(Protocol::kSize, roots().protocol_layout());
  result->set_methods(methods);
  result->set_super(super);
  result->set_name(name);
  IF_PARANOID(result->validate());
  return result;
}

allocation<Protocol> Heap::allocate_empty_protocol() {
  @alloc Protocol *result = allocate_object<Protocol>(Protocol::kSize, roots().protocol_layout());
  return result;
}

allocation<Method> Heap::new_method(Selector *selector, Signature *signature, Lambda *lambda) {
  @alloc Method *result = allocate_object<Method>(Method::kSize, roots().method_layout());
  result->set_selector(selector);
  result->set_signature(signature);
  result->set_lambda(lambda);
  IF_PARANOID(result->validate());
  return result;
}

allocation<Signature> Heap::new_signature(Tuple *parameters) {
  @alloc Signature *result = allocate_object<Signature>(Signature::kSize, roots().signature_layout());
  result->set_parameters(parameters);
  IF_PARANOID(result->validate());
  return result;
}

allocation<String> Heap::new_string(string value) {
  uword size = String::size_for(value.length());
  @alloc String *result = allocate_object<String>(size, roots().string_layout());
  result->set_length(value.length());
  for (uword i = 0; i < value.length(); i++)
    result->set(i, value[i]);
  @assert size == result->size_in_memory();
  IF_PARANOID(result->validate());
  return result;
}

allocation<Stack> Heap::new_stack(uword height) {
  uword size = Stack::size_for(height);
  @alloc Stack *result = allocate_object<Stack>(size, roots().stack_layout());
  result->set_height(height);
  result->set_fp(0);
  result->set_sp(0);
  result->set_pc(0);
  result->set_top_marker(0);
  result->set_status(Stack::Status());
  IF_PARANOID(result->validate());
  return result;
}

maybe<Task> Heap::new_task(Architecture &arch) {
  @alloc Stack *stack = new_stack(Stack::kInitialHeight);
  @alloc Task *result = allocate_object<Task>(Task::kSize, roots().task_layout());
  result->set_stack(stack);
  result->set_caller(roots().nuhll());
  IF_PARANOID(result->validate());
  @try arch.initialize_task(result);
  return result;
}

allocation<String> Heap::new_string(uword length) {
  uword size = String::size_for(length);
  @alloc String *result = allocate_object<String>(size, roots().string_layout());
  result->set_length(length);
  @assert size == result->size_in_memory();
  IF_PARANOID(result->validate());
  return result;
}

allocation<Code> Heap::new_code(uword size) {
  @alloc(AbstractBuffer) Code *result = new_abstract_buffer(sizeof(uint16_t) * size, roots().code_layout());
  IF_PARANOID(result->validate());
  return result;
}

allocation<Buffer> Heap::new_buffer(uword total_size) {
  @alloc(AbstractBuffer) Buffer *result = new_abstract_buffer(total_size, roots().buffer_layout());
  return result;
}

allocation<AbstractBuffer> Heap::new_abstract_buffer(uword byte_count, Layout *type) {
  uword size = AbstractBuffer::size_for(byte_count);
  @alloc AbstractBuffer *result = allocate_object<AbstractBuffer>(size, type);
  result->set_size<uint8_t>(byte_count);
  @assert size == result->size_in_memory();
  IF_PARANOID(result->validate());
  return result;
}

allocation<Tuple> Heap::new_tuple(uword length) {
  uword size = Tuple::size_for(length);
  @alloc Tuple *result = allocate_object<Tuple>(size, roots().tuple_layout());
  result->set_length(length);
  for (uword i = 0; i < length; i++)
    result->set(i, roots().nuhll());
  @assert size == result->size_in_memory();
  IF_PARANOID(result->validate());
  return result;
}

allocation<Array> Heap::new_array(uword length) {
  uword size = Array::size_for(length);
  @alloc Array *result = allocate_object<Array>(size, roots().array_layout());
  result->set_length(length);
  for (uword i = 0; i < length; i++)
    result->set(i, roots().nuhll());
  @assert size == result->size_in_memory();
  IF_PARANOID(result->validate());
  return result;
}

allocation<Symbol> Heap::new_symbol(Value *name) {
  @alloc Symbol *result = allocate_object<Symbol>(Symbol::kSize, roots().symbol_layout());
  result->set_name(name);
  result->set_data(roots().nuhll());
  IF_PARANOID(result->validate());
  return result;
}

allocation<LocalVariable> Heap::new_local_variable(Symbol *symbol) {
  @alloc LocalVariable *result = allocate_object<LocalVariable>(LocalVariable::kSize, roots().local_variable_layout());
  result->set_symbol(symbol);
  IF_PARANOID(result->validate());
  return result;
}

allocation<Singleton> Heap::new_singleton(Layout *type) {
  @alloc Singleton *result = allocate_object<Singleton>(Singleton::kSize, type);
  return result;
}

allocation<HashMap> Heap::new_hash_map() {
  @alloc Tuple *table = new_tuple(0);
  return new_hash_map(table);
}

allocation<HashMap> Heap::new_hash_map(Tuple *table) {
  @alloc HashMap *result = allocate_object<HashMap>(HashMap::kSize, roots().hash_map_layout());
  result->set_table(table);
  IF_PARANOID(result->validate());
  return result;
}

allocation<Instance> Heap::new_instance(InstanceLayout *layout) {
  @assert tInstance == layout->instance_type();
  uword field_count = layout->instance_field_count();
  uword size = Instance::size_for(field_count);
  @alloc Instance *result = allocate_object<Instance>(size, layout);
  for (uword i = 0; i < field_count; i++)
    result->set_field(i, roots().vhoid());
  IF_PARANOID(result->validate());
  return result;
}

allocation<Selector> Heap::new_selector(Immediate *name, Smi *argc, Bool *is_accessor) {
  @alloc Selector *result = allocate_object<Selector>(Selector::kSize, roots().selector_layout());
  result->set_name(name);
  result->set_argc(argc);
  result->set_keywords(roots().empty_tuple());
  result->set_is_accessor(is_accessor);
  IF_PARANOID(result->validate());
  return result;
}

allocation<Forwarder> Heap::new_forwarder(Forwarder::Type type, Value *target) {
  // Right now the only kind of forwarder that makes any sense are
  // open ones.
  @assert Forwarder::fwOpen == type;
  @alloc ForwarderDescriptor *desc = new_forwarder_descriptor(type, target);
  return Forwarder::to(desc);
}

allocation<ForwarderDescriptor> Heap::new_forwarder_descriptor(Forwarder::Type type, Value *target) {
  @alloc ForwarderDescriptor *result = allocate_object<ForwarderDescriptor>(ForwarderDescriptor::kSize, roots().forwarder_descriptor_layout());
  result->set_raw_type(type);
  result->set_target(target);
  IF_PARANOID(result->validate());
  return result;
}

} // neutrino
