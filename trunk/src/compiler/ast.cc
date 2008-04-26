#include "compiler/ast-inl.h"
#include "compiler/compiler.h"
#include "runtime/runtime-inl.h"

namespace neutrino {

// -------------------------
// --- C o m p i l i n g ---
// -------------------------

ref<Protocol> ref_traits<ProtocolExpression>::compile(Runtime &runtime,
    ref<Context> context) {
  ref<ProtocolExpression> self = open(this);
  Factory &factory = runtime.factory();
  ref<Tuple> method_asts = methods(runtime.refs());
  ref<Tuple> methods = factory.new_tuple(method_asts.length());
  for (uword i = 0; i < method_asts.length(); i++) {
    ref_scope scope(runtime.refs());
    ref<MethodExpression> method_ast = cast<MethodExpression>(method_asts.get(runtime.refs(), i));
    ref<Method> method = method_ast.compile(runtime, context);
    methods.set(i, method);
  }
  return factory.new_protocol(methods, super(runtime.refs()), name(runtime.refs()));
}

ref<Method> ref_traits<MethodExpression>::compile(Runtime &runtime, 
    ref<Context> context) {
  ref<MethodExpression> self = open(this);
  ref<Lambda> code = Compiler::compile(runtime, self.lambda(runtime.refs()), context);
  Factory &factory = runtime.factory();
  ref<Signature> signature = factory.new_signature(factory.new_tuple(0));
  ref<Method> result = factory.new_method(selector(runtime.refs()), signature, code);
  return result;
}

void Lambda::ensure_compiled(Runtime &runtime, Method *holder) {
  if (is<Code>(code())) return;
  ref_scope scope(runtime.refs());
  ref<Method> holder_ref = runtime.refs().new_ref(holder);
  ref<Lambda> self_ref = runtime.refs().new_ref(this);
  self_ref.ensure_compiled(runtime, holder_ref);
}

void ref_traits<Lambda>::ensure_compiled(Runtime &runtime, ref<Method> holder) {
  ref<Lambda> self = open(this);
  if (is<Code>(self->code())) return;
  Compiler::compile(runtime, self, holder);
}


// -------------------------
// --- U n p a r s i n g ---
// -------------------------

class QuoteScope {
public:
  QuoteScope(QuoteTemplate *obj, QuoteScope *parent)
      : obj_(obj)
      , parent_(parent) { }
  QuoteScope *parent() { return parent_; }
  QuoteTemplate *obj() { return obj_; }
private:
  QuoteTemplate *obj_;
  QuoteScope *parent_;
};

class UnparseData {
public:
  UnparseData(string_buffer &buf)
      : buf_(buf)
      , quote_scope_(NULL) { }
  string_buffer *operator->() { return &buf_; }
  string_buffer &out() { return buf_; }
  QuoteTemplate *current_quote_template();
  QuoteScope *quote_scope() { return quote_scope_; }
  void set_quote_scope(QuoteScope *scope) { quote_scope_ = scope; }
private:
  string_buffer &buf_;
  QuoteScope *quote_scope_;
};

QuoteTemplate *UnparseData::current_quote_template() {
  ASSERT(quote_scope() != NULL);
  return quote_scope()->obj();
}

static void unparse_syntax_tree_on(SyntaxTree *obj, UnparseData &buf);

static void unparse_literal_expression_on(LiteralExpression *obj,
    UnparseData &data) {
  obj->value()->write_on(data.out());
}

static void unparse_invoke_expression_on(InvokeExpression *obj, UnparseData &data) {
  unparse_syntax_tree_on(obj->receiver(), data);
  data->append('.');
  obj->selector()->write_on(data.out(), Data::UNQUOTED);
  data->append('(');
  bool is_first = true;
  Arguments *args = cast<Arguments>(obj->arguments());
  for (uword i = 0; i < args->arguments()->length(); i++) {
    if (is_first) is_first = false;
    else data->append(", ");
    unparse_syntax_tree_on(cast<SyntaxTree>(args->arguments()->get(i)), data);
  }
  data->append(')');
}

static void unparse_protocol_expression(ProtocolExpression *obj, UnparseData &data) {
  data->append("protocol ");
  obj->name()->write_on(data.out(), Data::UNQUOTED);
  data->append(" {");
  Tuple *methods = obj->methods();
  for (uword i = 0; i < methods->length(); i++) {
    MethodExpression *method = cast<MethodExpression>(methods->get(i));
    data->append(" ");
    unparse_syntax_tree_on(method, data);
  }
  data->append(" }");
}

static void unparse_return_expression(ReturnExpression *obj, UnparseData &data) {
  data->append("return ");
  unparse_syntax_tree_on(obj->value(), data);
}

static void unparse_method_expression(MethodExpression *obj, UnparseData &data) {
  data->append("def ");
  obj->selector()->write_on(data.out(), Data::UNQUOTED);
  unparse_syntax_tree_on(obj->lambda(), data);
  data->append(";");
}

static void unparse_symbol(Symbol *obj, UnparseData &data) {
  Value *name = obj->name();
  if (is<Void>(name)) {
    data->printf("&#%", reinterpret_cast<uword>(obj));
  } else {
    data->append("$");
    name->write_on(data.out(), Data::UNQUOTED);
  }
}

static void unparse_local_definition(LocalDefinition *obj, UnparseData &data) {
  data->append("def ");
  unparse_symbol(obj->symbol(), data);
  data->append(" := ");
  unparse_syntax_tree_on(obj->value(), data);
  data->append(" in ");
  unparse_syntax_tree_on(obj->body(), data);
}

static void unparse_list_on(Tuple *objs, UnparseData &data) {
  data->append("(");
  bool is_first = true;
  for (uword i = 0; i < objs->length(); i++) {
    if (is_first) is_first = false;
    else data->append(", ");
    unparse_syntax_tree_on(cast<SyntaxTree>(objs->get(i)), data);
  }
  data->append(")");
}

static void unparse_lambda_expression(LambdaExpression *obj, UnparseData &data) {
  data->append("fn ");
  if (is<Parameters>(obj->parameters()))
    unparse_list_on(cast<Parameters>(obj->parameters())->parameters(), data);
  data->append(" ");
  unparse_syntax_tree_on(obj->body(), data);
}

static void unparse_call_expression(CallExpression *obj, UnparseData &data) {
  unparse_syntax_tree_on(obj->receiver(), data);
  data->append("·");
  unparse_syntax_tree_on(obj->function(), data);
  unparse_list_on(obj->arguments()->arguments(), data);
}

static void unparse_unquote_expression(UnquoteExpression *obj, UnparseData &data) {
  data->printf("‹#%›", obj->index());
}

static void unparse_quote_template(QuoteTemplate *obj, UnparseData &data) {
  QuoteScope scope(obj, data.quote_scope());
  data.set_quote_scope(&scope);
  unparse_syntax_tree_on(obj->value(), data);
  data.set_quote_scope(scope.parent());
}

static void unparse_builtin_call(BuiltinCall *obj, UnparseData &data) {
  data->printf("bc[%]", obj->index());
}

static void unparse_global_variable(GlobalVariable *expr, UnparseData &data) {
  data->append("$");
  expr->name()->write_on(data.out(), Data::UNQUOTED);
}

static void unparse_local_variable(LocalVariable *expr, UnparseData &data) {
  data->append("$_");
  expr->symbol()->write_on(data.out(), Data::UNQUOTED);
}

static void unparse_sequence_expression(SequenceExpression *expr, UnparseData &data) {
  data->append("{");
  Tuple *exprs = expr->expressions();
  bool is_first = true;
  for (uword i = 0; i < exprs->length(); i++) {
    if (is_first) is_first = false;
    else data->append(", ");
    unparse_syntax_tree_on(cast<SyntaxTree>(exprs->get(i)), data);
  }
  data->append("}");
}

static void unparse_conditional_expression(ConditionalExpression *expr, UnparseData &data) {
  data->append("if (");
  unparse_syntax_tree_on(expr->condition(), data);
  data->append(") ");
  unparse_syntax_tree_on(expr->then_part(), data);
  data->append(" else ");
  unparse_syntax_tree_on(expr->else_part(), data);
}

static void unparse_assignment(Assignment *expr, UnparseData &data) {
  unparse_symbol(expr->symbol(), data);
  data->append(" := ");
  unparse_syntax_tree_on(expr->value(), data);
}

static void unparse_interpolate_expression(InterpolateExpression *expr,
    UnparseData &data) {
  data->append("#<interpolate>");
}

static void unparse_syntax_tree_on(SyntaxTree *obj, UnparseData &data) {
  InstanceType type = obj->type();
  switch (type) {
  case tLiteralExpression:
    unparse_literal_expression_on(cast<LiteralExpression>(obj), data);
    break;
  case tInvokeExpression:
    unparse_invoke_expression_on(cast<InvokeExpression>(obj), data);
    break;
  case tProtocolExpression:
    unparse_protocol_expression(cast<ProtocolExpression>(obj), data);
    break;
  case tMethodExpression:
    unparse_method_expression(cast<MethodExpression>(obj), data);
    break;
  case tReturnExpression:
    unparse_return_expression(cast<ReturnExpression>(obj), data);
    break;
  case tUnquoteExpression:
    unparse_unquote_expression(cast<UnquoteExpression>(obj), data);
    break;
  case tGlobalVariable:
    unparse_global_variable(cast<GlobalVariable>(obj), data);
    break;
  case tLocalVariable:
    unparse_local_variable(cast<LocalVariable>(obj), data);
    break;
  case tLocalDefinition:
    unparse_local_definition(cast<LocalDefinition>(obj), data);
    break;
  case tLambdaExpression:
    unparse_lambda_expression(cast<LambdaExpression>(obj), data);
    break;
  case tThisExpression:
    data->append("this");
    break;
  case tCallExpression:
    unparse_call_expression(cast<CallExpression>(obj), data);
    break;
  case tQuoteTemplate:
    unparse_quote_template(cast<QuoteTemplate>(obj), data);
    break;
  case tBuiltinCall:
    unparse_builtin_call(cast<BuiltinCall>(obj), data);
    break;
  case tSequenceExpression:
    unparse_sequence_expression(cast<SequenceExpression>(obj), data);
    break;
  case tAssignment:
    unparse_assignment(cast<Assignment>(obj), data);
    break;
  case tConditionalExpression:
    unparse_conditional_expression(cast<ConditionalExpression>(obj), data);
    break;
  case tInterpolateExpression:
    unparse_interpolate_expression(cast<InterpolateExpression>(obj), data);
    break;
  default:
    UNHANDLED(InstanceType, type);
  }
}

void SyntaxTree::unparse_on(string_buffer &buf) {
  UnparseData data(buf);
  unparse_syntax_tree_on(this, data);
}


// ---------------------
// --- V i s i t o r ---
// ---------------------

static void dispatch_single(Visitor &visitor, ref<Value> value, bool ignore_unexpected) {
  if (is<SyntaxTree>(value)) {
    cast<SyntaxTree>(value).accept(visitor);
  } else if (is<Symbol>(value)) {
    visitor.visit_symbol(cast<Symbol>(value));
  } else {
    ASSERT(ignore_unexpected);
  }
}


/**
 * Dispatch the visitor for the given value.  If the value is a
 * syntax tree node it is visited normally.  If it is a tuple, the
 * tuple is assumed to contain syntax tree nodes which are visited
 * in order.  Otherwise nothing happens.
 */
static void dispatch(Visitor &visitor, ref<Value> value, const char *field) {
  if (is<Tuple>(value)) {
    ref<Tuple> tuple = cast<Tuple>(value);
    ref_scope scope(visitor.refs());
    for (uword i = 0; i < tuple.length(); i++)
      dispatch_single(visitor, tuple.get(visitor.refs(), i), false);
  } else {
    dispatch_single(visitor, value, true);
  }
}


void ref_traits<SyntaxTree>::accept(Visitor &visitor) {
  ref<SyntaxTree> self = open(this);
  InstanceType type = self->type();
  switch (type) {
  case tQuoteTemplate: {
    QuoteTemplateScope scope(visitor, cast<QuoteTemplate>(self));
    return cast<QuoteTemplate>(self).value(visitor.refs()).accept(visitor);
  }
  case tUnquoteExpression: {
    ref<QuoteTemplate> templ = visitor.current_quote();
    uword index = cast<UnquoteExpression>(self)->index();
    Value *term = templ->unquotes()->get(index);
    ref<Value> value = visitor.refs().new_ref(term);    
    DropQuoteTemplateScope drop(visitor);
    return dispatch_single(visitor, value, false);
  }
#define MAKE_VISIT(n, Name, name)                                    \
  case t##Name:                                                      \
    return visitor.visit_##name(cast<Name>(self));
eAcceptVisitorCases(MAKE_VISIT)
#undef MAKE_VISIT
  default:
    UNHANDLED(InstanceType, type);
  }
}


/**
 * Dispatch the visitor for each element in the given tuple.
 */
static void dispatch_tuple(Visitor &visitor, ref<Tuple> tuple, const char *field) {
  ref_scope scope(visitor.refs());
  for (uword i = 0; i < tuple.length(); i++)
    dispatch(visitor, tuple.get(visitor.refs(), i), field);
}


void ref_traits<SyntaxTree>::traverse(Visitor &visitor) {
#define VISIT_FIELD_WITH(field, Type, op) op(visitor, cast<Type>(self).field(visitor.refs()), #Type "." #field);
#define VISIT_FIELD(ret, field, Field, Type) VISIT_FIELD_WITH(field, Type, dispatch)
  ref<SyntaxTree> self = open(this);
  InstanceType type = self->type();
  switch (type) {
  case tArguments: {
    ref_scope scope(visitor.refs());
    VISIT_FIELD(0, arguments, 0, Arguments);
    break;
  }
  case tInstantiateExpression: {
    ref_scope scope(visitor.refs());
    VISIT_FIELD(0, receiver, 0, InstantiateExpression);
    VISIT_FIELD(0, arguments, 0, InstantiateExpression);
    VISIT_FIELD_WITH(terms, InstantiateExpression, dispatch_tuple);
    break;
  }
  case tInterpolateExpression: {
    ref_scope scope(visitor.refs());
    VISIT_FIELD_WITH(terms, InterpolateExpression, dispatch_tuple);
    break;
  }
  case tParameters: {
    ref_scope scope(visitor.refs());
    VISIT_FIELD_WITH(parameters, Parameters, dispatch_tuple);
    break;
  }
  case tQuoteExpression: {
    ref_scope scope(visitor.refs());
    VISIT_FIELD(0, unquotes, 0, QuoteExpression);
    break;
  }
#define MAKE_VISIT(n, Name, name)                                    \
  case t##Name: {                                                    \
    ref_scope scope(visitor.refs());                                 \
    e##Name##Fields(VISIT_FIELD, Name);                              \
    break;                                                           \
  }
eTraverseVisitorCases(MAKE_VISIT)
#undef MAKE_VISIT    
  default:
    UNHANDLED(InstanceType, type);
  }
#undef VISIT_FIELD
}

Visitor::~Visitor() { }

void Visitor::visit_syntax_tree(ref<SyntaxTree> that) {
  that.traverse(*this);
}

void Visitor::visit_symbol(ref<Symbol> that) {
  // ignore
}

#define MAKE_VISIT_METHOD(n, Name, name)                             \
  void Visitor::visit_##name(ref<Name> that) {                       \
    visit_syntax_tree(that);                                         \
  }
eSyntaxTreeTypes(MAKE_VISIT_METHOD)
#undef MAKE_VISIT_METHOD

}
