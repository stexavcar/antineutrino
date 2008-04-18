#include "backends/bytecode.h"
#include "compiler/compile-utils-inl.h"
#include "main/options.h"
#include "runtime/interpreter-inl.h"
#include "utils/string-inl.h"
#include "values/values-inl.h"


namespace neutrino {


BytecodeBackend::BytecodeBackend(Runtime &runtime)
  : AbstractBackend(runtime)
  , pool_(runtime)
  , stack_height_(0) { }


void BytecodeBackend::invoke(ref<Selector> selector, uint16_t argc,
    ref<Tuple> keymap) {
  STATIC_CHECK(OpcodeInfo<ocInvoke>::kArgc == 3);
  uint16_t selector_index = constant_pool_index(selector);
  uint16_t keymap_index = constant_pool_index(keymap);
  code().append(ocInvoke);
  code().append(selector_index);
  code().append(argc);
  code().append(keymap_index);
}


void BytecodeBackend::invoke_super(ref<Selector> selector, uint16_t argc,
    ref<Tuple> keymap, ref<Signature> current) {
  STATIC_CHECK(OpcodeInfo<ocInvokeSuper>::kArgc == 4);
  uint16_t selector_index = constant_pool_index(selector);
  uint16_t keymap_index = constant_pool_index(keymap);
  uint16_t current_index = constant_pool_index(current);
  code().append(ocInvokeSuper);
  code().append(selector_index);
  code().append(argc);
  code().append(keymap_index);
  code().append(current_index);
}


void BytecodeBackend::instantiate(ref<Layout> layout) {
  STATIC_CHECK(OpcodeInfo<ocNew>::kArgc == 1);
  uint16_t layout_index = constant_pool_index(layout);
  code().append(ocNew);
  code().append(layout_index);
  adjust_stack_height(-static_cast<word>(layout->instance_field_count()));
}


void BytecodeBackend::raise(ref<String> name, uint16_t argc) {
  STATIC_CHECK(OpcodeInfo<ocRaise>::kArgc == 2);
  uint16_t name_index = constant_pool_index(name);
  code().append(ocRaise);
  code().append(name_index);
  code().append(argc);
}


void BytecodeBackend::call(uint16_t argc) {
  STATIC_CHECK(OpcodeInfo<ocCall>::kArgc == 1);
  code().append(ocCall);
  code().append(argc);
}


void BytecodeBackend::push(ref<Value> value) {
  STATIC_CHECK(OpcodeInfo<ocPush>::kArgc == 1);
  uint16_t index = constant_pool_index(value);
  code().append(ocPush);
  code().append(index);
  adjust_stack_height(1);
}


void BytecodeBackend::global(ref<Value> value) {
  STATIC_CHECK(OpcodeInfo<ocGlobal>::kArgc == 1);
  uint16_t index = constant_pool_index(value);
  code().append(ocGlobal);
  code().append(index);
  adjust_stack_height(1);
}


void BytecodeBackend::argument(uint16_t index) {
  STATIC_CHECK(OpcodeInfo<ocArgument>::kArgc == 1);
  code().append(ocArgument);
  code().append(index);
  adjust_stack_height(1);
}


void BytecodeBackend::keyword(uint16_t index) {
  STATIC_CHECK(OpcodeInfo<ocKeyword>::kArgc == 1);
  code().append(ocKeyword);
  code().append(index);
  adjust_stack_height(1);
}


void BytecodeBackend::load_local(uint16_t height) {
  STATIC_CHECK(OpcodeInfo<ocLoadLocal>::kArgc == 1);
  code().append(ocLoadLocal);
  code().append(height);
  adjust_stack_height(1);
}


void BytecodeBackend::store_local(uint16_t height) {
  STATIC_CHECK(OpcodeInfo<ocStoreLocal>::kArgc == 1);
  code().append(ocStoreLocal);
  code().append(height);
}


void BytecodeBackend::outer(uint16_t index) {
  STATIC_CHECK(OpcodeInfo<ocOuter>::kArgc == 1);
  code().append(ocOuter);
  code().append(index);
  adjust_stack_height(1);
}


void BytecodeBackend::closure(ref<Lambda> lambda, uint16_t outers) {
  STATIC_CHECK(OpcodeInfo<ocClosure>::kArgc == 2);
  uint16_t index = constant_pool_index(lambda);
  code().append(ocClosure);
  code().append(index);
  code().append(outers);
}


void BytecodeBackend::task() {
  STATIC_CHECK(OpcodeInfo<ocTask>::kArgc == 0);
  code().append(ocTask);
}


void BytecodeBackend::yield() {
  STATIC_CHECK(OpcodeInfo<ocYield>::kArgc == 0);
  code().append(ocYield);
}


void BytecodeBackend::pop(uint16_t height) {
  STATIC_CHECK(OpcodeInfo<ocPop>::kArgc == 1);
  code().append(ocPop);
  code().append(height);
  adjust_stack_height(-height);
}


void BytecodeBackend::slap(uint16_t height) {
  if (height == 0) return;
  STATIC_CHECK(OpcodeInfo<ocSlap>::kArgc == 1);
  code().append(ocSlap);
  code().append(height);
  adjust_stack_height(-height);
}


void BytecodeBackend::swap() {
  STATIC_CHECK(OpcodeInfo<ocSwap>::kArgc == 0);
  code().append(ocSwap);
}


void BytecodeBackend::rethurn() {
  STATIC_CHECK(OpcodeInfo<ocReturn>::kArgc == 0);
  ASSERT(stack_height() > 0);
  code().append(ocReturn);
}


void BytecodeBackend::attach() {
  STATIC_CHECK(OpcodeInfo<ocAttach>::kArgc == 0);
  code().append(ocAttach);
  adjust_stack_height(1);
}


void BytecodeBackend::tuple(uint16_t size) {
  STATIC_CHECK(OpcodeInfo<ocTuple>::kArgc == 1);
  code().append(ocTuple);
  code().append(size);
  adjust_stack_height(-(size - 1));
}


void BytecodeBackend::concat(uint16_t terms) {
  STATIC_CHECK(OpcodeInfo<ocConcat>::kArgc == 1);
  code().append(ocConcat);
  code().append(terms);
  adjust_stack_height(-(terms - 1));
}


void BytecodeBackend::quote(uint16_t unquotes) {
  STATIC_CHECK(OpcodeInfo<ocQuote>::kArgc == 1);
  ASSERT(unquotes > 0);
  code().append(ocQuote);
  code().append(unquotes);
  adjust_stack_height(-unquotes);
}


void BytecodeBackend::builtin(uint16_t argc, uint16_t index) {
  STATIC_CHECK(OpcodeInfo<ocBuiltin>::kArgc == 2);
  code().append(ocBuiltin);
  code().append(argc);
  code().append(index);
  adjust_stack_height(1);
}


void BytecodeBackend::if_true(BytecodeLabel &label) {
  STATIC_CHECK(OpcodeInfo<ocIfTrue>::kArgc == 1);
  code().append(ocIfTrue);
  code().append(label.value());
  if (!label.is_bound()) label.set_value(code().length() - 1);
}


void BytecodeBackend::if_false(BytecodeLabel &label) {
  STATIC_CHECK(OpcodeInfo<ocIfFalse>::kArgc == 1);
  code().append(ocIfFalse);
  code().append(label.value());
  if (!label.is_bound()) label.set_value(code().length() - 1);
}


void BytecodeBackend::ghoto(BytecodeLabel &label) {
  STATIC_CHECK(OpcodeInfo<ocGoto>::kArgc == 1);
  code().append(ocGoto);
  code().append(label.value());
  if (!label.is_bound()) label.set_value(code().length() - 1);
}


void BytecodeBackend::bind(BytecodeLabel &label) {
  ASSERT(!label.is_bound());
  uword value = code().length();
  uword current = label.value();
  label.set_value(value);
  while (current != BytecodeLabel::kNoTarget) {
    uword next = code()[current];
    code()[current] = value;
    current = next;
  }
}


void BytecodeBackend::mark(ref<Value> data) {
  STATIC_CHECK(OpcodeInfo<ocMark>::kArgc == 1);
  uint16_t index = constant_pool_index(data);
  code().append(ocMark);
  code().append(index);
  adjust_stack_height(Marker::kSize);
}


void BytecodeBackend::unmark() {
  STATIC_CHECK(OpcodeInfo<ocUnmark>::kArgc == 0);
  code().append(ocUnmark);  
  adjust_stack_height(-static_cast<word>(Marker::kSize));
}


void BytecodeBackend::new_forwarder(uint16_t type) {
  STATIC_CHECK(OpcodeInfo<ocForward>::kArgc == 1);
  code().append(ocForward);
  code().append(type);
}


void BytecodeBackend::bind_forwarder() {
  STATIC_CHECK(OpcodeInfo<ocBindForwarder>::kArgc == 0);
  code().append(ocBindForwarder);
  adjust_stack_height(-1);
}


ref<Code> BytecodeBackend::flush_code() {
  ref<Code> result = factory().new_code(code().length());
  for (uword i = 0; i < result.length(); i++)
    result.at(i) = code()[i];
  return result;
}


ref<Tuple> BytecodeBackend::flush_constant_pool() {
  ref<Tuple> result = factory().new_tuple(pool().length());
  for (uword i = 0; i < result.length(); i++)
    result->set(i, pool().get(i));
  return result;
}


uint16_t BytecodeBackend::constant_pool_index(ref<Value> value) {
  heap_list &pool = this->pool();
  for (uint16_t i = 0; i < pool.length(); i++) {
    ref_scope scope(refs());
    ref<Value> entry = pool[i];
    if (entry->is_identical(*value)) return i;
  }
  uint16_t result = pool.length();
  pool.append(value);
  return result;
}


ref<Method> BytecodeBackend::field_getter(uword index, 
    ref<Selector> selector, ref<Signature> signature,
    ref<Context> context) {
  ref<Code> ld_code = factory().new_code(4);
  STATIC_CHECK(OpcodeInfo<ocLoadField>::kArgc == 2);
  ld_code->at(0) = ocLoadField;
  ld_code->at(1) = index;
  ld_code->at(2) = 0;
  ld_code->at(3) = ocReturn;
  ref<Lambda> lambda = factory().new_lambda(0, ld_code,
      runtime().empty_tuple(), runtime().nuhll(), context);
  return factory().new_method(selector, signature, lambda);
}


ref<Method> BytecodeBackend::field_setter(uword index, 
    ref<Selector> selector, ref<Signature> signature,
    ref<Context> context) {
  ref<Code> st_code = factory().new_code(6);
  STATIC_CHECK(OpcodeInfo<ocStoreField>::kArgc == 2);
  STATIC_CHECK(OpcodeInfo<ocArgument>::kArgc == 1);
  st_code->at(0) = ocArgument;
  st_code->at(1) = 0;
  st_code->at(2) = ocStoreField;
  st_code->at(3) = index;
  st_code->at(4) = 1;
  st_code->at(5) = ocReturn;
  ref<Lambda> lambda = factory().new_lambda(1, st_code,
    runtime().empty_tuple(), runtime().nuhll(), context);
  return factory().new_method(selector, signature, lambda);
}


void BytecodeBackend::adjust_stack_height(word delta) {
  ASSERT_GE(stack_height() + delta, 0);
  stack_height_ += delta;
  if (Options::check_stack_height) {
    code().append(ocCheckHeight);
    code().append(stack_height());
  }
}


void BytecodeBackend::disassemble_next_instruction(uword *pc_ptr,
    vector<uint16_t> data, vector<Value*> pool, string_buffer &buf) {
  buf.printf("%{3} ", *pc_ptr);
  OpcodeData opcode_data;
  opcode_data.load(data[*pc_ptr]);
  ASSERT(opcode_data.is_resolved());
  buf.printf("%", opcode_data.name());
  string format = opcode_data.format();
  for (uword i = 0; i < format.length(); i++) {
    buf.append(" ");
    uint16_t arg = data[*pc_ptr + 1 + i];
    switch (format[i]) {
    case 'i':
      buf.printf("%", arg);
      break;
    case '@':
      buf.printf("@%", arg);
      break;
    case 'p':
      pool[arg]->write_on(buf);
      break;
    case '?':
      buf.append("?");
      break;
    case '.':
      break;
    default:
      UNREACHABLE();
    }
  }
  buf.append("\n");
  *pc_ptr += opcode_data.length();
}


void BytecodeArchitecture::run(ref<Lambda> lambda, ref<Task> task) {
  
  interpreter().call(lambda, task);
}


void BytecodeArchitecture::disassemble(Lambda *lambda, string_buffer &buf) {
  uword pc = 0;
  vector<uint16_t> code = cast<Code>(lambda->code())->buffer();
  uword code_length = cast<Code>(lambda->code())->length();
  vector<Value*> pool = cast<Tuple>(lambda->constant_pool())->buffer();
  while (pc < code_length)
    BytecodeBackend::disassemble_next_instruction(&pc, code, pool, buf);
}


} // neutrino
