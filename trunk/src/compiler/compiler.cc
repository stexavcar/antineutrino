#include "compiler/ast-inl.h"
#include "compiler/compiler.h"
#include "compiler/compile-utils-inl.h"
#include "heap/ref-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/list-inl.h"

namespace neutrino {

class Scope;
class Label;


// -------------------------
// --- A s s e m b l e r ---
// -------------------------

#define __ this->

class Assembler : public Visitor {
public:
  Assembler(Runtime &runtime)
      : runtime_(runtime)
      , pool_(runtime.factory())
      , scope_(NULL) { }
  void initialize() { pool().initialize(); }
  Scope &scope() { ASSERT(scope_ != NULL); return *scope_; }
  void set_scope(Scope &that) { scope_ = &that; }
  
  ref<Code> flush_code();
  ref<Tuple> flush_constant_pool();
  
  uint16_t constant_pool_index(ref<Value> value);
  
  void codegen(ref<SyntaxTree> that) { that.accept(*this); }
  void push(ref<Value> value);
  void pop(uint16_t height = 1);
  void slap(uint16_t height);
  void rethurn();
  void invoke(ref<String> name, uint16_t argc);
  void call(uint16_t argc);
  void tuple(uint32_t size);
  void global(ref<Value> name);
  void argument(uint16_t index);
  void if_true(Label &label);
  void ghoto(Label &label);
  void bind(Label &label);
  
  virtual void visit_syntax_tree(ref<SyntaxTree> that);
  virtual void visit_literal_expression(ref<LiteralExpression> that);
  virtual void visit_return_expression(ref<ReturnExpression> that);
  virtual void visit_sequence_expression(ref<SequenceExpression> that);
  virtual void visit_invoke_expression(ref<InvokeExpression> that);
  virtual void visit_tuple_expression(ref<TupleExpression> that);
  virtual void visit_global_expression(ref<GlobalExpression> that);
  virtual void visit_call_expression(ref<CallExpression> that);
  virtual void visit_symbol(ref<Symbol> that);
  virtual void visit_conditional_expression(ref<ConditionalExpression> that);

private:
  Factory &factory() { return runtime().factory(); }
  Runtime &runtime() { return runtime_; }
  heap_list &pool() { return pool_; }
  list_buffer<uint16_t> &code() { return code_; }
  Runtime &runtime_;
  list_buffer<uint16_t> code_;
  heap_list pool_;
  Scope *scope_;
};

class Label {
public:
  Label()
    : is_bound_(false)
    , value_(0) { }
  bool is_bound() { return is_bound_; }
  uint32_t value() { return value_; }
  void set_value(uint32_t addr) { value_ = addr; }
private:
  bool is_bound_;
  uint32_t value_;
};

ref<Code> Assembler::flush_code() {
  ref<Code> result = factory().new_code(code().length());
  for (uint32_t i = 0; i < result.length(); i++)
    result.at(i) = code()[i];
  return result;
}

ref<Tuple> Assembler::flush_constant_pool() {
  ref<Tuple> result = factory().new_tuple(pool().length());
  for (uint32_t i = 0; i < result.length(); i++)
    result->set(i, pool().get(i));
  return result;
}

uint16_t Assembler::constant_pool_index(ref<Value> value) {
  heap_list &pool = this->pool();
  for (uint16_t i = 0; i < pool.length(); i++) {
    RefScope scope;
    ref<Value> entry = pool[i];
    if (entry->equals(*value)) return i;
  }
  uint16_t result = pool.length();
  pool.append(value);
  return result;
}

void Assembler::invoke(ref<String> name, uint16_t argc) {
  STATIC_CHECK(OpcodeInfo<OC_INVOKE>::kArgc == 2);
  uint16_t name_index = constant_pool_index(name);
  code().append(OC_INVOKE);
  code().append(name_index);
  code().append(argc);
}

void Assembler::call(uint16_t argc) {
  STATIC_CHECK(OpcodeInfo<OC_CALL>::kArgc == 1);
  code().append(OC_CALL);
  code().append(argc);
}

void Assembler::push(ref<Value> value) {
  STATIC_CHECK(OpcodeInfo<OC_PUSH>::kArgc == 1);
  uint16_t index = constant_pool_index(value);
  code().append(OC_PUSH);
  code().append(index);
}

void Assembler::global(ref<Value> value) {
  STATIC_CHECK(OpcodeInfo<OC_GLOBAL>::kArgc == 1);
  uint16_t index = constant_pool_index(value);
  code().append(OC_GLOBAL);
  code().append(index);
}

void Assembler::argument(uint16_t index) {
  STATIC_CHECK(OpcodeInfo<OC_ARGUMENT>::kArgc == 1);
  code().append(OC_ARGUMENT);
  code().append(index);
}

void Assembler::pop(uint16_t height) {
  STATIC_CHECK(OpcodeInfo<OC_POP>::kArgc == 1);
  code().append(OC_POP);
  code().append(height);
}

void Assembler::slap(uint16_t height) {
  STATIC_CHECK(OpcodeInfo<OC_SLAP>::kArgc == 1);
  code().append(OC_SLAP);
  code().append(height);
}

void Assembler::rethurn() {
  STATIC_CHECK(OpcodeInfo<OC_RETURN>::kArgc == 0);
  code().append(OC_RETURN);
}

void Assembler::tuple(uint32_t size) {
  STATIC_CHECK(OpcodeInfo<OC_TUPLE>::kArgc == 1);
  code().append(OC_TUPLE);
  code().append(size);
}

void Assembler::if_true(Label &label) {
  STATIC_CHECK(OpcodeInfo<OC_IF_TRUE>::kArgc == 1);
  code().append(OC_IF_TRUE);
  code().append(label.value());
  if (!label.is_bound()) label.set_value(code().length() - 1);
}

void Assembler::ghoto(Label &label) {
  STATIC_CHECK(OpcodeInfo<OC_GOTO>::kArgc == 1);
  code().append(OC_GOTO);
  code().append(label.value());
  if (!label.is_bound()) label.set_value(code().length() - 1);
}

void Assembler::bind(Label &label) {
  ASSERT(!label.is_bound());
  uint32_t value = code().length();
  uint32_t current = label.value();
  label.set_value(value);
  while (current != 0) {
    uint32_t next = code()[current];
    code()[current] = value;
    current = next;
  }
}


// -----------------
// --- S c o p e ---
// -----------------

enum Category {
  MISSING, ARGUMENT
};

struct Lookup {
  Lookup() : category(MISSING) { }
  Category category;
  union {
    struct { uint16_t index; } argument_info;
  };  
};

class Scope {
public:
  Scope(ref<Tuple> symbols, Scope *parent)
      : symbols_(symbols)
      , parent_(parent) { }
  void lookup(ref<Symbol> symbol, Lookup &result);
private:
  ref<Tuple> symbols() { return symbols_; }
  ref<Tuple> symbols_;
  Scope *parent_;
};

void Scope::lookup(ref<Symbol> symbol, Lookup &result) {
  for (uint32_t i = 0; i < symbols().length(); i++) {
    if (symbol->equals(symbols()->at(i))) {
      result.category = ARGUMENT;
      result.argument_info.index = symbols().length() - i - 1;
      return;
    }
  }
  if (parent_ != NULL) return parent_->lookup(symbol, result);
}


// -------------------------------------
// --- C o d e   G e n e r a t i o n ---
// -------------------------------------

void Assembler::visit_syntax_tree(ref<SyntaxTree> that) {
  UNHANDLED(InstanceType, that.type());
}

void Assembler::visit_return_expression(ref<ReturnExpression> that) {
  __ codegen(that.value());
  __ rethurn();
}

void Assembler::visit_literal_expression(ref<LiteralExpression> that) {
  __ push(that.value());
}

void Assembler::visit_invoke_expression(ref<InvokeExpression> that) {
  RefScope scope;
  __ codegen(that.receiver());
  __ push(runtime().vhoid());
  ref<Tuple> args = that.arguments();
  for (uint32_t i = 0; i < args.length(); i++)
    __ codegen(cast<SyntaxTree>(args.get(i)));
  __ invoke(that.name(), args.length());
  __ slap(args.length() + 1);
}

void Assembler::visit_call_expression(ref<CallExpression> that) {
  RefScope scope;
  __ codegen(that.receiver());
  __ codegen(that.function());
  ref<Tuple> args = that.arguments();
  for (uint32_t i = 0; i < args.length(); i++)
    __ codegen(cast<SyntaxTree>(args.get(i)));
  __ call(args.length());
  __ slap(args.length() + 1);
}

void Assembler::visit_sequence_expression(ref<SequenceExpression> that) {
  RefScope scope;
  ref<Tuple> expressions = that.expressions();
  ASSERT(expressions.length() > 1);
  bool is_first = true;
  for (uint32_t i = 0; i < expressions.length(); i++) {
    if (is_first) {
      is_first = false;
    } else {
      __ pop();
    }
    __ codegen(cast<SyntaxTree>(expressions.get(i)));
  }
}

void Assembler::visit_tuple_expression(ref<TupleExpression> that) {
  RefScope scope;
  ref<Tuple> values = that.values();
  for (uint32_t i = 0; i < values.length(); i++)
    __ codegen(cast<SyntaxTree>(values.get(i)));
  __ tuple(values.length());
}

void Assembler::visit_global_expression(ref<GlobalExpression> that) {
  __ global(that.name());
}

void Assembler::visit_symbol(ref<Symbol> that) {
  Lookup lookup;
  scope().lookup(that, lookup);
  switch (lookup.category) {
    case ARGUMENT:
      __ argument(lookup.argument_info.index);
      break;
    default:
      UNREACHABLE();
  }
}

void Assembler::visit_conditional_expression(ref<ConditionalExpression> that) {
  Label then, end;
  __ codegen(that.condition());
  __ if_true(then);
  __ codegen(that.else_part());
  __ ghoto(end);
  __ bind(then);
  __ codegen(that.then_part());
  __ bind(end);
}


// -----------------------
// --- C o m p i l e r ---
// -----------------------

class CompileSession {
public:
  CompileSession(Runtime &runtime);
  ref<Lambda> compile(ref<MethodExpression> tree);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

CompileSession::CompileSession(Runtime &runtime)
    : runtime_(runtime) { }

ref<Lambda> Compiler::compile(ref<MethodExpression> method) {
  CompileSession session(Runtime::current());
  return session.compile(method);
}

ref<Lambda> CompileSession::compile(ref<MethodExpression> method) {
  Assembler assembler(runtime());
  ref<Tuple> params = method.params();
  Scope scope(params, NULL);
  assembler.set_scope(scope);
  assembler.initialize();
  method.body().accept(assembler);
  ref<Code> code = assembler.flush_code();
  ref<Tuple> constant_pool = assembler.flush_constant_pool();
  return runtime().factory().new_lambda(params.length(), code, constant_pool);
}

} // neutrino
