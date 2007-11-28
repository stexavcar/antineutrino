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
  Assembler(ref<LambdaExpression> lambda, Runtime &runtime)
      : lambda_(lambda)
      , runtime_(runtime)
      , pool_(runtime.factory())
      , scope_(NULL)
      , stack_height_(0) { }
  void initialize() { pool().initialize(); }
  Scope &scope() { ASSERT(scope_ != NULL); return *scope_; }
  
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
  void tuple(uint16_t size);
  void global(ref<Value> name);
  void argument(uint16_t index);
  void local(uint16_t height);
  void if_true(Label &label);
  void ghoto(Label &label);
  void bind(Label &label);
  void builtin(uint16_t argc, uint16_t index);
  void concat(uint16_t terms);
  
  virtual void visit_syntax_tree(ref<SyntaxTree> that);
#define MAKE_VISIT_METHOD(n, NAME, Name, name)                       \
  virtual void visit_##name(ref<Name> that);
FOR_EACH_SYNTAX_TREE_TYPE(MAKE_VISIT_METHOD)
#undef MAKE_VISIT_METHOD

private:
  static const bool kCheckStackHeight = false;
  friend class Scope;
  ref<LambdaExpression> lambda() { return lambda_; }
  Factory &factory() { return runtime().factory(); }
  uint32_t stack_height() { return stack_height_; }
  void adjust_stack_height(int32_t delta);
  Runtime &runtime() { return runtime_; }
  heap_list &pool() { return pool_; }
  list_buffer<uint16_t> &code() { return code_; }
  ref<LambdaExpression> lambda_;
  Runtime &runtime_;
  list_buffer<uint16_t> code_;
  heap_list pool_;
  Scope *scope_;
  uint32_t stack_height_;
};

class Label {
public:
  Label() : is_bound_(false) , value_(kNoTarget) { }
  bool is_bound() { return is_bound_; }
  uint32_t value() { return value_; }
  void set_value(uint32_t addr) { ASSERT(addr != kNoTarget); value_ = addr; }
  static const uint32_t kNoTarget = 0;
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
    if (entry->is_identical(*value)) return i;
  }
  uint16_t result = pool.length();
  pool.append(value);
  return result;
}

void Assembler::adjust_stack_height(int32_t delta) {
  stack_height_ += delta;
  if (kCheckStackHeight) {
    code().append(OC_CHKHGT);
    code().append(stack_height_);
  }
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
  adjust_stack_height(1);
}

void Assembler::global(ref<Value> value) {
  STATIC_CHECK(OpcodeInfo<OC_GLOBAL>::kArgc == 1);
  uint16_t index = constant_pool_index(value);
  code().append(OC_GLOBAL);
  code().append(index);
  adjust_stack_height(1);
}

void Assembler::argument(uint16_t index) {
  STATIC_CHECK(OpcodeInfo<OC_ARGUMENT>::kArgc == 1);
  code().append(OC_ARGUMENT);
  code().append(index);
  adjust_stack_height(1);
}

void Assembler::local(uint16_t height) {
  STATIC_CHECK(OpcodeInfo<OC_LOCAL>::kArgc == 1);
  code().append(OC_LOCAL);
  code().append(height);
  adjust_stack_height(1);
}

void Assembler::pop(uint16_t height) {
  STATIC_CHECK(OpcodeInfo<OC_POP>::kArgc == 1);
  code().append(OC_POP);
  code().append(height);
  adjust_stack_height(-height);
}

void Assembler::slap(uint16_t height) {
  STATIC_CHECK(OpcodeInfo<OC_SLAP>::kArgc == 1);
  code().append(OC_SLAP);
  code().append(height);
  adjust_stack_height(-height);
}

void Assembler::rethurn() {
  STATIC_CHECK(OpcodeInfo<OC_RETURN>::kArgc == 0);
  code().append(OC_RETURN);
}

void Assembler::tuple(uint16_t size) {
  STATIC_CHECK(OpcodeInfo<OC_TUPLE>::kArgc == 1);
  code().append(OC_TUPLE);
  code().append(size);
  adjust_stack_height(-(size - 1));
}

void Assembler::concat(uint16_t terms) {
  STATIC_CHECK(OpcodeInfo<OC_CONCAT>::kArgc == 1);
  code().append(OC_CONCAT);
  code().append(terms);
  adjust_stack_height(-(terms - 1));
}

void Assembler::builtin(uint16_t argc, uint16_t index) {
  STATIC_CHECK(OpcodeInfo<OC_BUILTIN>::kArgc == 2);
  code().append(OC_BUILTIN);
  code().append(argc);
  code().append(index);
  adjust_stack_height(1);
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
  while (current != Label::kNoTarget) {
    uint32_t next = code()[current];
    code()[current] = value;
    current = next;
  }
}


// -----------------
// --- S c o p e ---
// -----------------

enum Category {
  MISSING, ARGUMENT, LOCAL
};

struct Lookup {
  Lookup() : category(MISSING) { }
  Category category;
  union {
    struct { uint16_t index; } argument_info;
    struct { uint16_t height; } local_info;
  };  
};

class Scope {
public:
  Scope(Assembler &assembler);
  ~Scope();
  virtual void lookup(ref<Symbol> symbol, Lookup &result) = 0;
protected:
  Scope *parent() { return parent_; }
private:
  Assembler &assembler_;
  Scope *parent_;
};

Scope::Scope(Assembler &assembler) 
    : assembler_(assembler)
    , parent_(assembler.scope_) {
  assembler.scope_ = this;
}

Scope::~Scope() {
  assembler_.scope_ = parent_;
}

class ArgumentScope : public Scope {
public:
  ArgumentScope(Assembler &assembler, ref<Tuple> symbols)
      : Scope(assembler)
      , symbols_(symbols) { }
  virtual void lookup(ref<Symbol> symbol, Lookup &result);
private:
  ref<Tuple> symbols() { return symbols_; }
  ref<Tuple> symbols_;
};

void ArgumentScope::lookup(ref<Symbol> symbol, Lookup &result) {
  for (uint32_t i = 0; i < symbols().length(); i++) {
    if (symbol->equals(symbols()->at(i))) {
      result.category = ARGUMENT;
      result.argument_info.index = symbols().length() - i - 1;
      return;
    }
  }
  if (parent() != NULL) return parent()->lookup(symbol, result);
}

class LocalScope : public Scope {
public:
  LocalScope(Assembler &assembler, ref<Symbol> symbol, uint32_t height)
      : Scope(assembler)
      , symbol_(symbol)
      , height_(height) { }
  virtual void lookup(ref<Symbol> symbol, Lookup &result);
private:
  ref<Symbol> symbol_;
  uint32_t height_;
};

void LocalScope::lookup(ref<Symbol> symbol, Lookup &result) {
  if (symbol->equals(*symbol_)) {
    result.category = LOCAL;
    result.local_info.height = height_;
  } else if (parent() != NULL) {
    return parent()->lookup(symbol, result);
  }
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

void Assembler::visit_quote_expression(ref<QuoteExpression> that) {
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
      __ local(lookup.local_info.height);
      break;
  }
}

void Assembler::visit_conditional_expression(ref<ConditionalExpression> that) {
  Label then, end;
  __ codegen(that.condition());
  __ if_true(then);
  adjust_stack_height(-1);
  uint32_t height_before = stack_height();
  __ codegen(that.else_part());
  __ ghoto(end);
  __ bind(then);
  adjust_stack_height(-1);
  USE(height_before); ASSERT_EQ(height_before, stack_height());
  __ codegen(that.then_part());
  __ bind(end);
}

void Assembler::visit_interpolate_expression(ref<InterpolateExpression> that) {
  RefScope scope;
  ref<Tuple> terms = that.terms();
  for (uint32_t i = 0; i < terms.length(); i++) {
    ref<Value> entry = terms.get(i);
    if (is<String>(entry)) {
      __ push(entry);
    } else {
      __ codegen(cast<SyntaxTree>(entry));
      __ push(runtime().vhoid());
      __ invoke(factory().new_string("to_string"), 0);
      __ slap(1);
    }
  }
  __ concat(terms.length());
}

void Assembler::visit_local_definition(ref<LocalDefinition> that) {
  uint32_t height = stack_height();
  __ codegen(that.value());
  LocalScope scope(*this, that.symbol(), height);
  __ codegen(that.body());
  __ slap(1);
}

void Assembler::visit_this_expression(ref<ThisExpression> that) {
  __ argument(lambda()->params()->length() + 1);
}

void Assembler::visit_builtin_call(ref<BuiltinCall> that) {
  __ builtin(that->argc(), that->index());
}

void Assembler::visit_class_expression(ref<ClassExpression> that) {
  visit_syntax_tree(that);
}

void Assembler::visit_lambda_expression(ref<LambdaExpression> that) {
  visit_syntax_tree(that);
}

void Assembler::visit_method_expression(ref<MethodExpression> that) {
  visit_syntax_tree(that);
}

// -----------------------
// --- C o m p i l e r ---
// -----------------------

class CompileSession {
public:
  CompileSession(Runtime &runtime);
  void compile(ref<Lambda> tree);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

CompileSession::CompileSession(Runtime &runtime)
    : runtime_(runtime) { }

ref<Lambda> Compiler::compile(ref<MethodExpression> method) {
  Runtime &runtime = Runtime::current();
  CompileSession session(runtime);
  ref<LambdaExpression> lambda_expr = method.lambda();
  ref<Lambda> lambda = runtime.factory().new_lambda(
    lambda_expr->params()->length(),
    runtime.vhoid(),
    runtime.vhoid(),
    lambda_expr
  );
  return lambda;
}

void Compiler::compile(ref<Lambda> lambda) {
  CompileSession session(Runtime::current());
  session.compile(lambda);
}

void CompileSession::compile(ref<Lambda> lambda) {
  ref<LambdaExpression> tree = lambda.tree();
  Assembler assembler(tree, runtime());
  ref<Tuple> params = tree.params();
  ArgumentScope scope(assembler, params);
  assembler.initialize();
  tree.body().accept(assembler);
  ref<Code> code = assembler.flush_code();
  ref<Tuple> constant_pool = assembler.flush_constant_pool();
  lambda->set_code(*code);
  lambda->set_literals(*constant_pool);
}

} // neutrino
