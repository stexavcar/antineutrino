#include "compiler/ast-inl.h"
#include "compiler/compiler.h"
#include "compiler/compile-utils-inl.h"
#include "heap/memory-inl.h"
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

class CompileSession {
public:
  CompileSession(Runtime &runtime, ref<Context> context);
  ref<Lambda> compile(ref<LambdaExpression> that, Assembler *enclosing);
  void compile(ref<Lambda> tree, Assembler *enclosing);
  Runtime &runtime() { return runtime_; }
  ref<Context> context() { return context_; }
private:
  Runtime &runtime_;
  ref<Context> context_;
};

class Assembler : public Visitor {
public:
  Assembler(ref<LambdaExpression> lambda, CompileSession &session,
      Assembler *enclosing)
      : lambda_(lambda)
      , session_(session)
      , pool_(session.runtime().factory())
      , stack_height_(0) {
    if (enclosing == NULL) return;
    scope_ = enclosing->scope_;
    set_quote_scope(enclosing->quote_scope());
  }
  void initialize() { pool().initialize(); }
  Scope &scope() { ASSERT(scope_ != NULL); return *scope_; }
  SyntaxTree *resolve_unquote(UnquoteExpression *that);
  
  ref<Code> flush_code();
  ref<Tuple> flush_constant_pool();
  
  uint16_t constant_pool_index(ref<Value> value);
  void load_symbol(ref<Symbol> sym);
  
  void codegen(ref<SyntaxTree> that) { that.accept(*this); }
  void push(ref<Value> value);
  void pop(uint16_t height = 1);
  void slap(uint16_t height);
  void rethurn();
  void invoke(ref<Selector> selector, uint16_t argc);
  void instantiate(ref<Layout> layout);
  void raise(ref<String> name, uint16_t argc);
  void call(uint16_t argc);
  void tuple(uint16_t size);
  void global(ref<Value> name);
  void argument(uint16_t index);
  void local(uint16_t height);
  void outer(uint16_t index);
  void closure(ref<Lambda> lambda, uint16_t outers);
  void if_true(Label &label);
  void ghoto(Label &label);
  void bind(Label &label);
  void builtin(uint16_t argc, uint16_t index);
  void external(uint16_t argc, ref<String> name);
  void concat(uint16_t terms);
  void quote(uint16_t unquotes);
  void mark(ref<Value> data);
  void unmark();
  
  virtual void visit_syntax_tree(ref<SyntaxTree> that);
#define MAKE_VISIT_METHOD(n, NAME, Name, name)                       \
  virtual void visit_##name(ref<Name> that);
FOR_EACH_SYNTAX_TREE_TYPE(MAKE_VISIT_METHOD)
#undef MAKE_VISIT_METHOD

  Factory &factory() { return runtime().factory(); }

private:
  friend class Scope;
  ref<LambdaExpression> lambda() { return lambda_; }
  Runtime &runtime() { return session().runtime(); }
  CompileSession &session() { return session_; }
  uword stack_height() { return stack_height_; }
  void adjust_stack_height(word delta);
  heap_list &pool() { return pool_; }
  list_buffer<uint16_t> &code() { return code_; }
  ref<LambdaExpression> lambda_;
  CompileSession &session_;
  list_buffer<uint16_t> code_;
  heap_list pool_;
  Scope *scope_;
  uword stack_height_;
};

class Label {
public:
  Label() : is_bound_(false) , value_(kNoTarget) { }
  bool is_bound() { return is_bound_; }
  uword value() { return value_; }
  void set_value(uword addr) { ASSERT(addr != kNoTarget); value_ = addr; }
  static const uword kNoTarget = 0;
private:
  bool is_bound_;
  uword value_;
};

SyntaxTree *Assembler::resolve_unquote(UnquoteExpression *expr) {
  ref<QuoteTemplate> templ = current_quote();
  uword index = expr->index();
  Value *term = templ->unquotes()->get(index);
  return cast<SyntaxTree>(term);
}

ref<Code> Assembler::flush_code() {
  ref<Code> result = factory().new_code(code().length());
  for (uword i = 0; i < result.length(); i++)
    result.at(i) = code()[i];
  return result;
}

ref<Tuple> Assembler::flush_constant_pool() {
  ref<Tuple> result = factory().new_tuple(pool().length());
  for (uword i = 0; i < result.length(); i++)
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

void Assembler::adjust_stack_height(word delta) {
  ASSERT_GE(stack_height() + delta, 0);
  stack_height_ += delta;
  IF_PARANOID(code().append(OC_CHKHGT));
  IF_PARANOID(code().append(stack_height()));
}

void Assembler::invoke(ref<Selector> selector, uint16_t argc) {
  STATIC_CHECK(OpcodeInfo<OC_INVOKE>::kArgc == 2);
  uint16_t selector_index = constant_pool_index(selector);
  code().append(OC_INVOKE);
  code().append(selector_index);
  code().append(argc);
}

void Assembler::instantiate(ref<Layout> layout) {
  STATIC_CHECK(OpcodeInfo<OC_NEW>::kArgc == 1);
  uint16_t layout_index = constant_pool_index(layout);
  code().append(OC_NEW);
  code().append(layout_index);
  adjust_stack_height(-layout->instance_field_count());
}

void Assembler::raise(ref<String> name, uint16_t argc) {
  STATIC_CHECK(OpcodeInfo<OC_RAISE>::kArgc == 2);
  uint16_t name_index = constant_pool_index(name);
  code().append(OC_RAISE);
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

void Assembler::outer(uint16_t index) {
  STATIC_CHECK(OpcodeInfo<OC_OUTER>::kArgc == 1);
  code().append(OC_OUTER);
  code().append(index);
  adjust_stack_height(1);
}

void Assembler::closure(ref<Lambda> lambda, uint16_t outers) {
  STATIC_CHECK(OpcodeInfo<OC_CLOSURE>::kArgc == 2);
  uint16_t index = constant_pool_index(lambda);
  code().append(OC_CLOSURE);
  code().append(index);
  code().append(outers);
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
  ASSERT(stack_height() > 0);
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

void Assembler::quote(uint16_t unquotes) {
  STATIC_CHECK(OpcodeInfo<OC_QUOTE>::kArgc == 1);
  ASSERT(unquotes > 0);
  code().append(OC_QUOTE);
  code().append(unquotes);
  adjust_stack_height(-unquotes);
}

void Assembler::builtin(uint16_t argc, uint16_t index) {
  STATIC_CHECK(OpcodeInfo<OC_BUILTIN>::kArgc == 2);
  code().append(OC_BUILTIN);
  code().append(argc);
  code().append(index);
  adjust_stack_height(1);
}

void Assembler::external(uint16_t argc, ref<String> name) {
  STATIC_CHECK(OpcodeInfo<OC_EXTERNAL>::kArgc == 2);
  code().append(OC_EXTERNAL);
  code().append(argc);
  uint16_t name_index = constant_pool_index(name);
  code().append(name_index);
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
  uword value = code().length();
  uword current = label.value();
  label.set_value(value);
  while (current != Label::kNoTarget) {
    uword next = code()[current];
    code()[current] = value;
    current = next;
  }
}

void Assembler::mark(ref<Value> data) {
  STATIC_CHECK(OpcodeInfo<OC_MARK>::kArgc == 1);
  uint16_t index = constant_pool_index(data);
  code().append(OC_MARK);
  code().append(index);
  adjust_stack_height(Marker::kSize);
}

void Assembler::unmark() {
  STATIC_CHECK(OpcodeInfo<OC_UNMARK>::kArgc == 0);
  code().append(OC_UNMARK);  
  adjust_stack_height(-Marker::kSize);
}


// -----------------
// --- S c o p e ---
// -----------------

enum Category {
  MISSING, ARGUMENT, LOCAL, OUTER
};

struct Lookup {
  Lookup(Assembler &assm)
      : category(MISSING)
      , assembler(assm) { }
  Category category;
  Assembler &assembler;
  union {
    struct { uint16_t index; } argument_info;
    struct { uint16_t height; } local_info;
    struct { uint16_t index; } outer_info;
  };  
};

class Scope {
public:
  Scope(Assembler &assembler);
  Scope();
  ~Scope();
  virtual void lookup(ref<Symbol> symbol, Lookup &result) = 0;
  void unlink();
protected:
  Scope *parent() { return parent_; }
private:
  Assembler *assembler() { return assembler_; }
  Assembler *assembler_;
  Scope *parent_;
};

Scope::Scope(Assembler &assembler) 
    : assembler_(&assembler)
    , parent_(assembler.scope_) {
  assembler.scope_ = this;
}

Scope::Scope() : assembler_(NULL), parent_(NULL) { }

Scope::~Scope() {
  unlink();
}

void Scope::unlink() {
  if (assembler() != NULL) {
    assembler()->scope_ = parent();
    assembler_ = NULL;
  }
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
  for (uword i = 0; i < symbols().length(); i++) {
    Value *entry = symbols()->get(i);
    if (is<UnquoteExpression>(entry))
      entry = result.assembler.resolve_unquote(cast<UnquoteExpression>(entry));
    if (symbol->equals(cast<Symbol>(entry))) {
      result.category = ARGUMENT;
      result.argument_info.index = symbols().length() - i - 1;
      return;
    }
  }
  if (parent() != NULL) return parent()->lookup(symbol, result);
}

class LocalScope : public Scope {
public:
  LocalScope(Assembler &assembler, ref<Symbol> symbol, uword height)
      : Scope(assembler)
      , symbol_(symbol)
      , height_(height) { }
  virtual void lookup(ref<Symbol> name, Lookup &result);
private:
  ref<Symbol> symbol() { return symbol_; }
  uword height() { return height_; }
  ref<Symbol> symbol_;
  uword height_;
};

void LocalScope::lookup(ref<Symbol> name, Lookup &result) {
  if (name->equals(*symbol())) {
    result.category = LOCAL;
    result.local_info.height = height();
  } else if (parent() != NULL) {
    return parent()->lookup(name, result);
  }
}

class ClosureScope : public Scope {
public:
  ClosureScope(Assembler &assembler, Factory &factory)
      : Scope(assembler)
      , outers_(factory) {
    outers().initialize();
  }
  virtual void lookup(ref<Symbol> symbol, Lookup &result);
  heap_list &outers() { return outers_; }
private:
  heap_list outers_;
};

void ClosureScope::lookup(ref<Symbol> symbol, Lookup &result) {
  for (uword i = 0; i < outers().length(); i++) {
    if (outers().get(i)->equals(*symbol)) {
      result.category = OUTER;
      result.outer_info.index = i;
      return;
    }
  }
  result.category = OUTER;
  result.outer_info.index = outers().length();
  outers().append(symbol);
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
  if (that->unquotes()->length() > 0) {
    RefScope scope;
    ref<Tuple> unquotes = that.unquotes();
    for (uword i = 0; i < unquotes->length(); i++) {
      __ codegen(cast<SyntaxTree>(unquotes.get(i)));
    }
    __ quote(unquotes->length());
  }
}

void Assembler::visit_invoke_expression(ref<InvokeExpression> that) {
  RefScope scope;
  __ codegen(that.receiver());
  __ push(runtime().vhoid());
  ref<Tuple> args = that.arguments().arguments();
  for (uword i = 0; i < args.length(); i++)
    __ codegen(cast<SyntaxTree>(args.get(i)));
  __ invoke(that.selector(), args.length());
  __ slap(args.length() + 1);
}

void Assembler::visit_call_expression(ref<CallExpression> that) {
  RefScope scope;
  __ codegen(that.receiver());
  __ codegen(that.function());
  ref<Tuple> args = that.arguments().arguments();
  for (uword i = 0; i < args.length(); i++)
    __ codegen(cast<SyntaxTree>(args.get(i)));
  __ call(args.length());
  __ slap(args.length() + 1);
}

void Assembler::visit_sequence_expression(ref<SequenceExpression> that) {
  RefScope scope;
  ref<Tuple> expressions = that.expressions();
  ASSERT(expressions.length() > 1);
  bool is_first = true;
  for (uword i = 0; i < expressions.length(); i++) {
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
  for (uword i = 0; i < values.length(); i++)
    __ codegen(cast<SyntaxTree>(values.get(i)));
  __ tuple(values.length());
}

void Assembler::visit_global_expression(ref<GlobalExpression> that) {
  __ global(that.name());
}

void Assembler::load_symbol(ref<Symbol> that) {
  Lookup lookup(*this);
  scope().lookup(that, lookup);
  switch (lookup.category) {
    case ARGUMENT:
      __ argument(lookup.argument_info.index);
      break;
    case LOCAL:
      __ local(lookup.local_info.height);
      break;
    case OUTER:
      __ outer(lookup.outer_info.index);
      break;
    default:
      UNREACHABLE();
  }
}

void Assembler::visit_symbol(ref<Symbol> that) {
  load_symbol(that);
}

void Assembler::visit_conditional_expression(ref<ConditionalExpression> that) {
  Label then, end;
  __ codegen(that.condition());
  __ if_true(then);
  adjust_stack_height(-1);
  uword height_before = stack_height();
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
  for (uword i = 0; i < terms.length(); i++) {
    ref<Value> entry = terms.get(i);
    if (is<String>(entry)) {
      __ push(entry);
    } else {
      __ codegen(cast<SyntaxTree>(entry));
      __ push(runtime().vhoid());
      ref<String> name = factory().new_string("to_string");
      ref<Selector> selector = factory().new_selector(name, Smi::from_int(0));
      __ invoke(selector, 0);
      __ slap(1);
    }
  }
  __ concat(terms.length());
}

void Assembler::visit_local_definition(ref<LocalDefinition> that) {
  uword height = stack_height();
  __ codegen(that.value());
  LocalScope scope(*this, that.symbol(), height);
  __ codegen(that.body());
  __ slap(1);
}

void Assembler::visit_lambda_expression(ref<LambdaExpression> that) {
  ClosureScope scope(*this, factory());
  ref<Lambda> lambda = session().compile(that, this);
  scope.unlink();
  if (scope.outers().length() > 0) {
    for (uword i = 0; i < scope.outers().length(); i++) {
      ref<Symbol> sym = cast<Symbol>(scope.outers()[i]);
      load_symbol(sym);
    }
    __ closure(lambda, scope.outers().length());
  } else {
    __ push(lambda);
  }
}

void Assembler::visit_this_expression(ref<ThisExpression> that) {
  __ argument(lambda()->params()->length() + 1);
}

void Assembler::visit_builtin_call(ref<BuiltinCall> that) {
  __ builtin(that->argc(), that->index());
}

void Assembler::visit_external_call(ref<ExternalCall> that) {
  __ external(that->argc()->value(), that.name());
}

void Assembler::visit_do_on_expression(ref<DoOnExpression> that) {
  RefScope scope;
  ref<Tuple> clauses = that.clauses();
  ref<Tuple> data = factory().new_tuple(2 * clauses.length());
  for (uword i = 0; i < clauses.length(); i++) {
    ref<OnClause> clause = cast<OnClause>(clauses.get(i));
    ref<String> name = clause.name();
    data.set(2 * i, name);
    ref<LambdaExpression> handler = clause.lambda();
    ClosureScope scope(*this, factory());
    ref<Lambda> lambda = session().compile(handler, this);
    scope.unlink();
    // TODO(5): We need a lifo block mechanism to implement outers in
    //   condition handlers
    ASSERT_EQ(0, scope.outers().length());
    data.set(2 * i + 1, lambda);
  }
  __ mark(data);
  __ codegen(that.value());
  __ unmark();
}

void Assembler::visit_raise_expression(ref<RaiseExpression> that) {
  RefScope scope;
  ref<Tuple> args = that.arguments().arguments();
  __ push(runtime().vhoid()); // receiver
  __ push(runtime().vhoid()); // method
  for (uword i = 0; i < args.length(); i++)
    __ codegen(cast<SyntaxTree>(args.get(i)));
  __ raise(that.name(), args.length());
  __ slap(args.length() + 1);
}

void Assembler::visit_instantiate_expression(ref<InstantiateExpression> that) {
  RefScope scope;
  ref<Tuple> terms = that.terms();
  uword term_count = terms.length() / 2;
  ref<Tuple> methods = factory().new_tuple(term_count);
  ref<Signature> signature = factory().new_signature(factory().new_tuple(0));
  __ codegen(that.receiver());
  for (uword i = 0; i < term_count; i++) {
    ref<String> keyword = cast<String>(terms.get(2 * i));
    ref<Code> code = factory().new_code(4);
    STATIC_CHECK(OpcodeInfo<OC_FIELD>::kArgc == 2);
    code->at(0) = OC_FIELD;
    code->at(1) = i;
    code->at(2) = 0;
    code->at(3) = OC_RETURN;
    ref<Lambda> lambda = factory().new_lambda(0, code, runtime().empty_tuple(),
        runtime().nuhll(), session().context());
    ref<Selector> selector = factory().new_selector(keyword, Smi::from_int(0));
    methods.set(i, factory().new_method(selector, signature, lambda));
    ref<SyntaxTree> value = cast<SyntaxTree>(terms.get(2 * i + 1));
    __ codegen(value);
  }
  ref<Layout> layout = factory().new_layout(INSTANCE_TYPE, term_count,
      runtime().vhoid(), methods);
  __ instantiate(layout);
}

void Assembler::visit_protocol_expression(ref<ProtocolExpression> that) {
  visit_syntax_tree(that);
}

void Assembler::visit_method_expression(ref<MethodExpression> that) {
  visit_syntax_tree(that);
}

void Assembler::visit_on_clause(ref<OnClause> that) {
  // This type of node is handled by the enclosing do-on expression.
  UNREACHABLE();
}

void Assembler::visit_unquote_expression(ref<UnquoteExpression> that) {
  // This is handled specially by the visitor and should never be
  // visited explicitly
  UNREACHABLE();
}

void Assembler::visit_quote_template(ref<QuoteTemplate> that) {
  // This is handled specially by the visitor and should never be
  // visited explicitly
  UNREACHABLE();
}

void Assembler::visit_arguments(ref<Arguments> that) {
  UNREACHABLE();
}

// -----------------------
// --- C o m p i l e r ---
// -----------------------

CompileSession::CompileSession(Runtime &runtime, ref<Context> context)
    : runtime_(runtime), context_(context) { }

ref<Lambda> Compiler::compile(ref<LambdaExpression> expr, ref<Context> context) {
  Runtime &runtime = Runtime::current();
  ref<Smi> zero = new_ref(Smi::from_int(0));  
  ref<Lambda> lambda = runtime.factory().new_lambda(
    expr->params()->length(), zero, zero, expr, context
  );
  return lambda;
}

ref<Lambda> Compiler::compile(ref<SyntaxTree> tree, ref<Context> context) {
  // TODO(2): Fix this once handles can be returned from scopes
  Lambda *result;
  Runtime &runtime = Runtime::current();
  {
    RefScope scope;
    ref<ReturnExpression> ret = runtime.factory().new_return_expression(tree);
    ref<LambdaExpression> expr = runtime.factory().new_lambda_expression(
      runtime.empty_tuple(),
      ret
    );
    ref<Lambda> ref_result = compile(expr, context);
    result = *ref_result;
  }
  return new_ref(result);
}

void Compiler::compile(ref<Lambda> lambda) {
  CompileSession session(Runtime::current(), lambda.context());
  session.compile(lambda, NULL);
}

ref<Lambda> CompileSession::compile(ref<LambdaExpression> that,
    Assembler *enclosing) {
  ref<Smi> zero = new_ref(Smi::from_int(0));
  ref<Lambda> lambda = runtime().factory().new_lambda(
      that->params()->length(), zero, zero, that, context()
  );
  compile(lambda, enclosing);
  return lambda;
}

void CompileSession::compile(ref<Lambda> lambda, Assembler *enclosing) {
  GarbageCollectionMonitor monitor(Runtime::current().heap().memory());
  ref<LambdaExpression> tree = cast<LambdaExpression>(lambda.tree());
  Assembler assembler(tree, *this, enclosing);
  ref<Tuple> params = tree.params();
  ArgumentScope scope(assembler, params);
  assembler.initialize();
  tree.body().accept(assembler);
  ref<Code> code = assembler.flush_code();
  ref<Tuple> constant_pool = assembler.flush_constant_pool();
  lambda->set_code(*code);
  lambda->set_constant_pool(*constant_pool);
  ASSERT(!monitor.has_collected_garbage());
}

} // neutrino
