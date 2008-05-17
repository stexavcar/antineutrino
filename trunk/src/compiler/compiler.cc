#include "compiler/ast-inl.h"
#include "compiler/compiler.h"
#include "compiler/compile-utils-inl.h"
#include "heap/memory-inl.h"
#include "heap/ref-inl.h"
#include "runtime/builtins.h"
#include "runtime/runtime-inl.h"
#include "utils/list-inl.h"
#include "utils/scoped-ptrs-inl.h"

namespace neutrino {

class Scope;


// -------------------------
// --- A s s e m b l e r ---
// -------------------------


#define __ backend().


class CompileSession {
public:
  CompileSession(Runtime &runtime, ref<Context> context);
  ref<Lambda> compile(ref<LambdaExpression> that, CodeGenerator &enclosing);
  void compile(ref<Lambda> tree, ref<Method> holder, CodeGenerator *enclosing);
  void compile(ref<Lambda> tree, ref<Method> holder);
  Runtime &runtime() { return runtime_; }
  ref<Context> context() { return context_; }
private:
  Runtime &runtime_;
  ref<Context> context_;
};


class CodeGenerator : public Visitor {
public:
  CodeGenerator(ref<LambdaExpression> lambda, ref<Method> method,
      CompileSession &session, CodeGenerator *enclosing)
    : Visitor(session.runtime().refs(), enclosing)
    , lambda_(lambda)
    , method_(method)
    , session_(session) { }

  SyntaxTree *resolve_unquote(UnquoteExpression *that);
  
  ref<Method> method() { return method_; }
  Runtime &runtime() { return session().runtime(); }

  /**
   * Attributes used to determine which code should be generated for
   * local variables.
   */
  enum LocalType {
    ltNone = 0,
    ltMaterialize = 1
  };
  
protected:
  ref<LambdaExpression> lambda() { return lambda_; }
  CompileSession &session() { return session_; }

private:

  ref<LambdaExpression> lambda_;
  ref<Method> method_;
  CompileSession &session_;
};


template <class Config>
class Assembler : public CodeGenerator {
public:
  Assembler(ref<LambdaExpression> lambda, ref<Method> method,
      CompileSession &session, typename Config::Backend &backend,
      CodeGenerator *enclosing)
      : CodeGenerator(lambda, method, session, enclosing)
      , backend_(backend) {
  }

  typedef void (Assembler<Config>::*special_builtin)();
  typedef typename Config::Label Label;
  typedef typename Config::Backend Backend;

  void load_raw_symbol(ref<Symbol> sym);
  void load_symbol(ref<Symbol> sym);
  void store_symbol(ref<Symbol> sym);
  
  void codegen(ref<SyntaxTree> that) {
    IF_DEBUG(uword height_before = backend().stack_height());
    that.accept(*this);
    ASSERT_EQ(height_before + 1, backend().stack_height());
  }

  special_builtin get_special(uword index);
  
#define MAKE_CASE(n, name, str) void name();
eSpecialBuiltinFunctions(MAKE_CASE)
#undef MAKE_CASE

  virtual void visit_syntax_tree(ref<SyntaxTree> that);
#define MAKE_VISIT_METHOD(n, Name, name)                             \
  virtual void visit_##name(ref<Name> that);
eSyntaxTreeTypes(MAKE_VISIT_METHOD)
#undef MAKE_VISIT_METHOD

  Factory &factory() { return runtime().factory(); }
  Backend &backend() { return backend_; }

private:
  Backend &backend_;
};


SyntaxTree *CodeGenerator::resolve_unquote(UnquoteExpression *expr) {
  UNREACHABLE();
  ref<QuoteTemplate> templ = current_quote();
  uword index = expr->index();
  Value *term = templ->unquotes()->get(index);
  return cast<SyntaxTree>(term);
}


class SymbolInfo : public Smi {
public:
  inline CodeGenerator::LocalType type();
  inline SymbolInfo *clone_with_type(CodeGenerator::LocalType value);
  inline LocalDefinition::Type def_type();
  inline SymbolInfo *clone_with_def_type(LocalDefinition::Type value);
  static inline SymbolInfo *empty() { return SymbolInfo::from(Smi::from_int(0)); }
  static inline SymbolInfo *from(Data *value);
};


SymbolInfo *SymbolInfo::from(Data *value) {
  if (is<Null>(value)) return empty();
  return static_cast<SymbolInfo*>(cast<Smi>(value));
}


#define BIT_FIELD_ACCESSORS(type, Class, name, start, size)          \
  type Class::name() {                                               \
    word mask = ((1 << size) - 1);                                   \
    return static_cast<type>((value() >> start) & mask);             \
  }                                                                  \
                                                                     \
  Class *Class::clone_with_##name(type val) {                        \
    word mask = ~(((1 << size) - 1) << start);                       \
    return Class::from(Smi::from_int((value() & mask) | (val << start))); \
  }


BIT_FIELD_ACCESSORS(CodeGenerator::LocalType, SymbolInfo, type, 0, 1)
BIT_FIELD_ACCESSORS(LocalDefinition::Type, SymbolInfo, def_type, 1, 3)


// -----------------
// --- S c o p e ---
// -----------------


#define eCategories(VISIT)                                           \
  VISIT(Missing)  VISIT(Argument) VISIT(Local)    VISIT(Outer)       \
  VISIT(Keyword)

enum Category {
  __first_category
#define MAKE_ENUM_ENTRY(Name) , c##Name
eCategories(MAKE_ENUM_ENTRY)
#undef MAKE_ENUM_ENTRY
};

MAKE_ENUM_INFO_HEADER(Category)
#define MAKE_ENTRY(Name) MAKE_ENUM_INFO_ENTRY(c##Name)
eCategories(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()


struct Lookup {
  Lookup() : category(cMissing) { }
  Category category;
  union {
    struct { uint16_t index; } argument_info;
    struct { uint16_t height; } local_info;
    struct { uint16_t index; } outer_info;
    struct { uint16_t index; } keyword_info;
  };  
};


class Scope {
public:
  Scope(Visitor &visitor);
  Scope();
  ~Scope();
  virtual void lookup(ref<Symbol> symbol, Lookup &result) = 0;
  void unlink();
protected:
  Scope *parent() { return parent_; }
private:
  Visitor *visitor() { return visitor_; }
  Visitor *visitor_;
  Scope *parent_;
};


Scope::Scope(Visitor &visitor) 
    : visitor_(&visitor)
    , parent_(visitor.scope_) {
  visitor.scope_ = this;
}


Scope::Scope() : visitor_(NULL), parent_(NULL) { }


Scope::~Scope() {
  unlink();
}


void Scope::unlink() {
  if (visitor() != NULL) {
    visitor()->scope_ = parent();
    visitor_ = NULL;
  }
}


class FunctionScope : public Scope {
public:
  FunctionScope(Runtime &runtime, Visitor &visitor,
      ref<Parameters> params, bool is_local,
      QuoteTemplateScope *quote_scope);
  virtual void lookup(ref<Symbol> symbol, Lookup &result);
private:
  Runtime &runtime() { return runtime_; }
  ref<Parameters> params() { return params_; }
  bool is_local() { return is_local_; }
  QuoteTemplateScope *quote_scope() { return quote_scope_; }
  Runtime &runtime_;
  ref<Parameters> params_;
  bool is_local_;
  QuoteTemplateScope *quote_scope_;
};


FunctionScope::FunctionScope(Runtime &runtime, Visitor &visitor,
    ref<Parameters> params, bool is_local,
    QuoteTemplateScope *quote_scope)
    : Scope(visitor)
    , runtime_(runtime)
    , params_(params)
    , is_local_(is_local)
    , quote_scope_(quote_scope) { }


void FunctionScope::lookup(ref<Symbol> symbol, Lookup &result) {
  Tuple *symbols = params()->parameters();
  // If we're looking up the 'this' symbol the result is the first
  // argument
  if (!is_local() && symbol == runtime().this_symbol()) {
    result.category = cArgument;
    result.argument_info.index = symbols->length();
    return;
  }
  // If it's not 'this' then maybe it's a parameter
  for (uword i = 0; i < symbols->length(); i++) {
    Value *entry = symbols->get(i);
    if (is<UnquoteExpression>(entry)) {
      uword index = cast<UnquoteExpression>(entry)->index();
      entry = quote_scope()->value()->unquotes()->get(index);
    }
    if (symbol->equals(cast<Symbol>(entry))) {
      uword posc = params()->position_count()->value();
      if (i < posc) {
        result.category = cArgument;
        result.argument_info.index = symbols->length() - i - 1;
        return;
      } else {
        result.category = cKeyword;
        result.keyword_info.index = i - posc;
        return;
      }
    }
  }
  // Don't know the symbol -- ask outside
  if (parent() != NULL) return parent()->lookup(symbol, result);
}


class LocalScope : public Scope {
public:
  LocalScope(Visitor &visitor, ref<Symbol> symbol, uword height)
      : Scope(visitor)
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
    result.category = cLocal;
    result.local_info.height = height();
  } else if (parent() != NULL) {
    return parent()->lookup(name, result);
  }
}


class ClosureScope : public Scope {
public:
  ClosureScope(Visitor &visitor, Runtime &runtime)
      : Scope(visitor)
      , outers_(runtime) {
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
      result.category = cOuter;
      result.outer_info.index = i;
      return;
    }
  }
  result.category = cOuter;
  result.outer_info.index = outers().length();
  outers().append(symbol);
}

// -------------------------------------
// --- C o d e   G e n e r a t i o n ---
// -------------------------------------

template <class C>
void Assembler<C>::visit_syntax_tree(ref<SyntaxTree> that) {
  UNHANDLED(InstanceType, that.type());
}

template <class C>
void Assembler<C>::visit_return_expression(ref<ReturnExpression> that) {
  codegen(that.value(refs()));
  __ rethurn();
}

template <class C>
void Assembler<C>::visit_yield_expression(ref<YieldExpression> that) {
  codegen(that.value(refs()));
  __ yield();
}

template <class C>
void Assembler<C>::visit_literal_expression(ref<LiteralExpression> that) {
  __ push(that.value(refs()));
}

template <class C>
void Assembler<C>::visit_quote_expression(ref<QuoteExpression> that) {
  __ push(that.value(refs()));
  if (that->unquotes()->length() > 0) {
    ref_scope scope(refs());
    ref<Tuple> unquotes = that.unquotes(refs());
    for (uword i = 0; i < unquotes->length(); i++) {
      codegen(cast<SyntaxTree>(unquotes.get(refs(), i)));
    }
    __ quote(unquotes->length());
  }
}

template <class C>
void Assembler<C>::visit_invoke_expression(ref<InvokeExpression> that) {
  ref_scope scope(refs());
  ref<SyntaxTree> recv = that.receiver(refs());
  bool is_super = is<SuperExpression>(recv);
  if (is_super) {
    codegen(cast<SuperExpression>(recv).value(refs()));
  } else {
    codegen(recv);
  }
  ref<Arguments> args_obj = cast<Arguments>(that.arguments(refs()));
  ref<Tuple> args = args_obj.arguments(refs());
  for (uword i = 0; i < args.length(); i++)
    codegen(cast<SyntaxTree>(args.get(refs(), i)));
  ref<Tuple> raw_keymap = args_obj.keyword_indices(refs());
  ref<Tuple> keymap;
  // The construction of the concrete keymap will eventually be moved
  // to runtime once we introduce optional keywords with default
  // values, since then we won't know which arguments are expected
  // by the method.
  if (raw_keymap.is_empty()) {
    keymap = raw_keymap;
  } else {
    keymap = runtime().factory().new_tuple(raw_keymap.length());
    uword argc = args.length();
    uword posc = args.length() - raw_keymap.length();
    for (uword i = 0; i < keymap.length(); i++) {
      uword index = cast<Smi>(raw_keymap->get(i))->value();
      keymap->set(i, Smi::from_int(argc - index - 1 - posc));
    }
  }
  if (is_super) {
    ref<Signature> current = method().signature(refs());
    __ invoke_super(that.selector(refs()), args.length(), keymap, current);
  } else {
    __ invoke(that.selector(refs()), args.length(), keymap);
  }
  __ slap(args.length());
}

template <class C>
void Assembler<C>::visit_call_expression(ref<CallExpression> that) {
  ref_scope scope(refs());
  codegen(that.receiver(refs()));
  codegen(that.function(refs()));
  __ swap();
  ref<Tuple> args = that.arguments(refs()).arguments(refs());
  for (uword i = 0; i < args.length(); i++)
    codegen(cast<SyntaxTree>(args.get(refs(), i)));
  __ call(args.length());
  __ slap(args.length() + 1);
}

template <class C>
void Assembler<C>::visit_sequence_expression(ref<SequenceExpression> that) {
  ref_scope scope(refs());
  ref<Tuple> expressions = that.expressions(refs());
  ASSERT(expressions.length() > 1);
  bool is_first = true;
  for (uword i = 0; i < expressions.length(); i++) {
    if (is_first) is_first = false;
    else __ pop();
    codegen(cast<SyntaxTree>(expressions.get(refs(), i)));
  }
}


template <class C>
void Assembler<C>::visit_tuple_expression(ref<TupleExpression> that) {
  ref_scope scope(refs());
  ref<Tuple> values = that.values(refs());
  for (uword i = 0; i < values.length(); i++)
    codegen(cast<SyntaxTree>(values.get(refs(), i)));
  __ tuple(values.length());
}


template <class C>
void Assembler<C>::visit_global_variable(ref<GlobalVariable> that) {
  __ global(that.name(refs()));
}


/**
 * Loads a "raw" symbol, which means that if this symbol has been
 * materialized as a referenced the raw reference is loaded but not
 * dereferenced.
 */
template <class C>
void Assembler<C>::load_raw_symbol(ref<Symbol> that) {
  Lookup lookup;
  scope().lookup(that, lookup);
  switch (lookup.category) {
    case cArgument:
      __ argument(lookup.argument_info.index);
      break;
    case cLocal:
      __ load_local(lookup.local_info.height);
      break;
    case cOuter:
      __ outer(lookup.outer_info.index);
      break;
    case cKeyword:
      __ keyword(lookup.keyword_info.index);
      break;
    default:
      that.to_string().println();
      UNHANDLED(Category, lookup.category);
  }
}


template <class C>
void Assembler<C>::load_symbol(ref<Symbol> that) {
  load_raw_symbol(that);
  if (SymbolInfo::from(that->data())->type() == ltMaterialize)
    __ load_cell();
}


template <class C>
void Assembler<C>::store_symbol(ref<Symbol> that) {
  if (SymbolInfo::from(that->data())->type() == ltMaterialize) {
    load_raw_symbol(that);
    __ store_cell();
  } else {
    Lookup lookup;
    scope().lookup(that, lookup);
    switch (lookup.category) {
      case cLocal:
        __ store_local(lookup.local_info.height);
        break;
      default:
        UNHANDLED(Category, lookup.category);
    }
  }
}


template <class C>
void Assembler<C>::visit_local_variable(ref<LocalVariable> that) {
  load_symbol(that.symbol(refs()));
}


template <class C>
void Assembler<C>::visit_this_expression(ref<ThisExpression> that) {
  load_symbol(runtime().this_symbol());
}


template <class C>
void Assembler<C>::visit_assignment(ref<Assignment> that) {
  codegen(that.value(refs()));
  store_symbol(that.symbol(refs()));
}


template <class C>
void Assembler<C>::visit_conditional_expression(ref<ConditionalExpression> that) {
  Label then, end;
  codegen(that.condition(refs()));
  __ if_true(then);
  backend().adjust_stack_height(-1);
  IF_DEBUG(uword height_before = backend().stack_height());
  codegen(that.else_part(refs()));
  __ ghoto(end);
  __ bind(then);
  backend().adjust_stack_height(-1);
  ASSERT_EQ(height_before, backend().stack_height());
  codegen(that.then_part(refs()));
  __ bind(end);
}


template <class C>
void Assembler<C>::visit_while_expression(ref<WhileExpression> that) {
  Label start, end;
  __ bind(start);
  codegen(that.condition(refs()));
  __ if_false(end);
  backend().adjust_stack_height(-1);
  codegen(that.body(refs()));
  __ pop();
  __ ghoto(start);
  __ bind(end);
  __ push(runtime().vhoid());
}


template <class C>
void Assembler<C>::visit_interpolate_expression(ref<InterpolateExpression> that) {
  ref_scope scope(refs());
  ref<Tuple> terms = that.terms(refs());
  for (uword i = 0; i < terms.length(); i++) {
    ref<Value> entry = terms.get(refs(), i);
    if (is<String>(entry)) {
      __ push(entry);
    } else {
      codegen(cast<SyntaxTree>(entry));
      ref<String> name = factory().new_string("to_string");
      ref<Selector> selector = factory().new_selector(name, Smi::from_int(0), runtime().fahlse());
      __ invoke(selector, 0, runtime().empty_tuple());
    }
  }
  __ concat(terms.length());
}


template <class C>
void Assembler<C>::visit_local_definition(ref<LocalDefinition> that) {
  Smi *type = that->type();
  if (type == Smi::from_int(LocalDefinition::ldRec)) {
    uword height = backend().stack_height();
    __ push(runtime().vhoid());
    __ new_forwarder(Forwarder::fwOpen);
    LocalScope scope(*this, that.symbol(refs()), height);
    codegen(that.value(refs()));
    __ bind_forwarder();
    codegen(that.body(refs()));
    __ slap(1);
  } else if (type == Smi::from_int(LocalDefinition::ldLoc)) {
    uword height = backend().stack_height();
    codegen(that.value(refs()));
    __ new_forwarder(Forwarder::fwOpen);
    LocalScope scope(*this, that.symbol(refs()), height);
    codegen(that.body(refs()));
    __ load_local(height);
    __ push(runtime().nuhll());
    __ bind_forwarder();
    __ pop(1);
    __ slap(1);
  } else {
    ASSERT(type == Smi::from_int(LocalDefinition::ldDef) || type == Smi::from_int(LocalDefinition::ldVar));
    uword height = backend().stack_height();
    codegen(that.value(refs()));
    if (SymbolInfo::from(that->symbol()->data())->type() == ltMaterialize) {
      ASSERT_EQ(type, Smi::from_int(LocalDefinition::ldVar));
      __ new_cell();
    }
    LocalScope scope(*this, that.symbol(refs()), height);
    codegen(that.body(refs()));
    __ slap(1);
  }
  // Clear any temporary data from the symbol
  that->symbol()->set_data(runtime().roots().nuhll());
}


template <class C>
void Assembler<C>::visit_lambda_expression(ref<LambdaExpression> that) {
  ClosureScope scope(*this, runtime());
  ref<Lambda> lambda = session().compile(that, *this);
  scope.unlink();
  if (scope.outers().length() > 0) {
    for (uword i = 0; i < scope.outers().length(); i++) {
      ref<Symbol> sym = cast<Symbol>(scope.outers()[i]);
      // Load raw symbols so that materialized variables are
      // transferred properly by reference rather than by value
      load_raw_symbol(sym);
    }
    __ closure(lambda, scope.outers().length());
  } else {
    __ push(lambda);
  }
}


template <class C>
void Assembler<C>::visit_task_expression(ref<TaskExpression> that) {
  codegen(that.lambda(refs()));
  __ task();
}


template <class C>
void Assembler<C>::visit_super_expression(ref<SuperExpression> that) {
  UNREACHABLE();
}


template <class C>
void Assembler<C>::visit_builtin_call(ref<BuiltinCall> that) {
  uword index = that->index();
  special_builtin special = get_special(index);
  if (special == NULL) {
    // This is a plain runtime builtin
    __ builtin(that->argc(), that->index());
  } else {
    (this->*special)();
  }
}


template <class C>
void Assembler<C>::visit_do_on_expression(ref<DoOnExpression> that) {
  ref_scope scope(refs());
  ref<Tuple> clauses = that.clauses(refs());
  ref<Tuple> data = factory().new_tuple(2 * clauses.length());
  for (uword i = 0; i < clauses.length(); i++) {
    ref<OnClause> clause = cast<OnClause>(clauses.get(refs(), i));
    ref<String> name = clause.name(refs());
    data.set(2 * i, name);
    ref<LambdaExpression> handler = clause.lambda(refs());
    ClosureScope scope(*this, runtime());
    ref<Lambda> lambda = session().compile(handler, *this);
    scope.unlink();
    // TODO(5): We need a lifo block mechanism to implement outers in
    //   condition handlers
    ASSERT_EQ(0, scope.outers().length());
    data.set(2 * i + 1, lambda);
  }
  __ mark(data);
  codegen(that.value(refs()));
  __ unmark();
}


template <class C>
void Assembler<C>::visit_raise_expression(ref<RaiseExpression> that) {
  ref_scope scope(refs());
  ref<Tuple> args = that.arguments(refs()).arguments(refs());
  __ push(runtime().vhoid()); // receiver
  for (uword i = 0; i < args.length(); i++)
    codegen(cast<SyntaxTree>(args.get(refs(), i)));
  __ raise(that.name(refs()), args.length());
  __ slap(args.length());
}


template <class C>
void Assembler<C>::visit_instantiate_expression(ref<InstantiateExpression> that) {
  ref_scope scope(refs());
  ref<Tuple> terms = that.terms(refs());
  uword term_count = terms.length() / 2;
  ref<Tuple> methods = factory().new_tuple(2 * term_count);
  ref<Signature> signature = factory().new_signature(factory().new_tuple(0));
  codegen(that.receiver(refs()));
  for (uword i = 0; i < term_count; i++) {
    ref<String> ld_keyword = cast<String>(terms.get(refs(), 2 * i));

    // Construct getter method for this field
    ref<Selector> get_selector = factory().new_selector(ld_keyword,
        Smi::from_int(0), runtime().thrue());
    ref<Method> getter = backend().field_getter(i, get_selector, signature, session().context());
    methods.set(2 * i, getter);
    
    // Construct setter method for this field
    own_vector<char> raw_name(ld_keyword.c_str());
    string_buffer st_name;
    st_name.append(raw_name.start());
    st_name.append(":=");
    ref<String> st_keyword = runtime().factory().new_string(st_name.raw_string());
    ref<Selector> set_selector = factory().new_selector(st_keyword, Smi::from_int(1), runtime().thrue());
    ref<Method> setter = backend().field_setter(i, set_selector, signature, session().context());
    methods.set(2 * i + 1, setter);
    
    // Load (initial) value for the field
    ref<SyntaxTree> value = cast<SyntaxTree>(terms.get(refs(), 2 * i + 1));
    codegen(value);
  }
  ref<Layout> layout = factory().new_layout(tInstance, term_count,
      runtime().vhoid(), methods);
  __ instantiate(layout);
}


template <class C>
void Assembler<C>::visit_protocol_expression(ref<ProtocolExpression> that) {
  visit_syntax_tree(that);
}


template <class C>
void Assembler<C>::visit_method_expression(ref<MethodExpression> that) {
  visit_syntax_tree(that);
}


template <class C>
void Assembler<C>::visit_on_clause(ref<OnClause> that) {
  // This type of node is handled by the enclosing do-on expression.
  UNREACHABLE();
}


template <class C>
void Assembler<C>::visit_unquote_expression(ref<UnquoteExpression> that) {
  // This is handled specially by the visitor and should never be
  // visited explicitly
  UNREACHABLE();
}


template <class C>
void Assembler<C>::visit_quote_template(ref<QuoteTemplate> that) {
  // This is handled specially by the visitor and should never be
  // visited explicitly
  UNREACHABLE();
}


template <class C>
void Assembler<C>::visit_arguments(ref<Arguments> that) {
  UNREACHABLE();
}


template <class C>
void Assembler<C>::visit_parameters(ref<Parameters> that) {
  UNREACHABLE();
}


// -----------------------
// --- B u i l t i n s ---
// -----------------------


template <class C>
void Assembler<C>::attach_task() {
  __ attach();
}


template <class C>
typename Assembler<C>::special_builtin Assembler<C>::get_special(uword index) {
  // This method is defined within the class because I don't know
  // how to declare the return type outside the class declaration
  switch (index) {
  #define MAKE_CASE(n, name, str) case n: return &Assembler::name;
  eSpecialBuiltinFunctions(MAKE_CASE)
  #undef MAKE_CASE
      default:
        return NULL;
    }
}


// -----------------------
// --- C o m p i l e r ---
// -----------------------

CompileSession::CompileSession(Runtime &runtime, ref<Context> context)
    : runtime_(runtime), context_(context) { }


ref<Lambda> Compiler::compile(Runtime &runtime, ref<LambdaExpression> expr,
    ref<Context> context) {
  ref<Smi> zero = runtime.refs().new_ref(Smi::from_int(0));  
  ref<Lambda> lambda = runtime.factory().new_lambda(
    expr->parameters()->parameters()->length(), zero, zero, expr, context
  );
  return lambda;
}


ref<Lambda> Compiler::compile(Runtime &runtime, ref<SyntaxTree> tree,
    ref<Context> context) {
  Lambda *result;
  {
    ref_scope scope(runtime.refs());
    ref<ReturnExpression> ret = runtime.factory().new_return_expression(tree);
    ref<Parameters> params = runtime.factory().new_parameters(
      runtime.refs().new_ref(Smi::from_int(0)),
      runtime.empty_tuple()
    );
    ref<LambdaExpression> expr = runtime.factory().new_lambda_expression(
      params,
      ret,
      false
    );
    ref<Lambda> ref_result = compile(runtime, expr, context);
    result = *ref_result;
  }
  return runtime.refs().new_ref(result);
}


void Compiler::compile(Runtime &runtime, ref<Lambda> lambda, ref<Method> holder) {
  CompileSession session(runtime, lambda.context(runtime.refs()));
  session.compile(lambda, holder);
}


ref<Lambda> CompileSession::compile(ref<LambdaExpression> that,
    CodeGenerator &enclosing) {
  ref<Smi> zero = runtime().refs().new_ref(Smi::from_int(0));
  ref<Lambda> lambda = runtime().factory().new_lambda(
      that->parameters()->parameters()->length(), zero, zero, that,
      context()
  );
  compile(lambda, enclosing.method(), &enclosing);
  return lambda;
}


// --- P r e   C o m p i l a t i o n   A n a l y s i s ---


/**
 * Analyzer that is invoked on the complete syntax tree before
 * code generation.  Is currently used to determine which local
 * variables are used by closures, so these variables can be
 * materialized.
 */
class Analyzer : public Visitor {
public:
  Analyzer(Runtime &runtime)
      : Visitor(runtime.refs(), NULL)
      , runtime_(runtime) { }
  void materialize_if_outer(ref<Symbol> sym);
  virtual void visit_assignment(ref<Assignment> that);
  virtual void visit_local_variable(ref<LocalVariable> that);
  virtual void visit_local_definition(ref<LocalDefinition> that);
  virtual void visit_lambda_expression(ref<LambdaExpression> that);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};


void Analyzer::materialize_if_outer(ref<Symbol> sym) {
  SymbolInfo *info = SymbolInfo::from(sym->data());
  if (info->type() == CodeGenerator::ltMaterialize) return;
  Lookup lookup;
  scope().lookup(sym, lookup);
  if (lookup.category == cOuter) {
    sym->set_data(info->clone_with_type(CodeGenerator::ltMaterialize));
  }  
}


void Analyzer::visit_assignment(ref<Assignment> that) {
  materialize_if_outer(that.symbol(refs()));
  Visitor::visit_assignment(that);
}


void Analyzer::visit_local_variable(ref<LocalVariable> that) {
  ref<Symbol> sym = that.symbol(refs());
  SymbolInfo *info = SymbolInfo::from(sym->data());
  if (info->def_type() == LocalDefinition::ldVar)
    materialize_if_outer(sym);
}


void Analyzer::visit_local_definition(ref<LocalDefinition> that) {
  ref<Symbol> sym = that.symbol(refs());
  LocalScope scope(*this, sym, 0);
  ASSERT_IS(Null, sym->data());
  if (that->type() == Smi::from_int(LocalDefinition::ldVar))
    sym->set_data(SymbolInfo::empty()->clone_with_def_type(LocalDefinition::ldVar));
  Visitor::visit_local_definition(that);
}


void Analyzer::visit_lambda_expression(ref<LambdaExpression> that) {
  ClosureScope scope(*this, runtime());
  Visitor::visit_lambda_expression(that);
}


// --- P o s t ---


/**
 * The post compilation checker traverses the whole syntax tree
 * and checks that it has been returned to a consistent state after
 * compilation.
 */
class PostCompilationChecker : public Visitor {
public:
  PostCompilationChecker(Runtime &runtime)
      : Visitor(runtime.refs(), NULL)
      , runtime_(runtime) { }
  virtual void visit_local_definition(ref<LocalDefinition> that);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};


void PostCompilationChecker::visit_local_definition(ref<LocalDefinition> that) {
  ASSERT_IS(Null, that->symbol()->data());
  Visitor::visit_local_definition(that);
}


} // neutrino


#include "backends/bytecode.h"


namespace neutrino {


void CompileSession::compile(ref<Lambda> lambda, ref<Method> holder) {
  GarbageCollectionMonitor monitor(runtime().heap().memory());
  ref<SyntaxTree> body = cast<LambdaExpression>(lambda.tree(runtime().refs())).body(runtime().refs());
  
  Analyzer analyzer(runtime());
  body.accept(analyzer);
  
  // Generate code
  compile(lambda, holder, NULL);
  
#ifdef PARANOID
  // Ensure that the syntax tree has been cleaned up propertly
  PostCompilationChecker checker(runtime());
  body.accept(checker);
#endif
  
  ASSERT(!monitor.has_collected_garbage());
}


void CompileSession::compile(ref<Lambda> lambda, ref<Method> holder,
    CodeGenerator *enclosing) {
  ref<LambdaExpression> tree = cast<LambdaExpression>(lambda.tree(runtime().refs()));
  BytecodeBackend backend(runtime());
  backend.initialize();
  Assembler<BytecodeCodeGeneratorConfig> assembler(tree, holder, *this, backend, enclosing);
  ref<Parameters> params = tree.parameters(runtime().refs());
  // If this is a keyword call the keyword map will be pushed as the
  // first local variable by the caller
  if (params->has_keywords())
    backend.adjust_stack_height(1);
  FunctionScope scope(runtime(), assembler, params,
      tree->is_local() == runtime().roots().thrue(),
      enclosing ? enclosing->quote_scope() : NULL);
  tree.body(runtime().refs()).accept(assembler);
  ref<Code> code = backend.flush_code();
  ref<Tuple> constant_pool = backend.flush_constant_pool();
  lambda->set_code(*code);
  lambda->set_constant_pool(*constant_pool);
}


} // neutrino
