#ifndef _IO_AST
#define _IO_AST

#include "values/values.h"

namespace neutrino {

// -----------------------------
// --- S y n t a x   T r e e ---
// -----------------------------

class SyntaxTree : public Object {
public:
  inline Value *&at(uword index);

  void unparse_on(string_buffer &buf);

  static const int kHeaderSize = Object::kHeaderSize;
};

template <>
class ref_traits<SyntaxTree> : public ref_traits<Object> {
public:
  void accept(Visitor &visitor);
  void traverse(Visitor &visitor);
};

DEFINE_REF_CLASS(SyntaxTree);


// -------------------------------------------
// --- L i t e r a l   E x p r e s s i o n ---
// -------------------------------------------

#define FOR_EACH_LITERAL_EXPRESSION_FIELD(VISIT, arg)                \
  VISIT(Value, value, Value, arg)

class LiteralExpression : public SyntaxTree {
public:
  FOR_EACH_LITERAL_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<LiteralExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_LITERAL_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(LiteralExpression);

// -----------------
// --- Q u o t e ---
// -----------------

#define FOR_EACH_QUOTE_TEMPLATE_FIELD(VISIT, arg)                    \
  VISIT(SyntaxTree, value,    Value,    arg)                         \
  VISIT(Tuple,      unquotes, Unquotes, arg)

class QuoteTemplate : public SyntaxTree {
public:
  FOR_EACH_QUOTE_TEMPLATE_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kUnquotesOffset = kValueOffset + kPointerSize;
  static const uword kSize = kUnquotesOffset + kPointerSize;
};

template <>
class ref_traits<QuoteTemplate> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_QUOTE_TEMPLATE_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(QuoteTemplate);


// -----------------------------------------
// --- I n v o k e   E x p r e s s i o n ---
// -----------------------------------------

#define FOR_EACH_INVOKE_EXPRESSION_FIELD(VISIT, arg)                 \
  VISIT(SyntaxTree, receiver,  Receiver,  arg)                       \
  VISIT(Selector,   selector,  Selector,  arg)                       \
  VISIT(Arguments,  arguments, Arguments, arg)

class InvokeExpression : public SyntaxTree {
public:
  FOR_EACH_INVOKE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kReceiverOffset  = SyntaxTree::kHeaderSize;
  static const uword kSelectorOffset  = kReceiverOffset + kPointerSize;
  static const uword kArgumentsOffset = kSelectorOffset + kPointerSize;
  static const uword kSize            = kArgumentsOffset + kPointerSize;
};

template <>
class ref_traits<InvokeExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_INVOKE_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(InvokeExpression);


// -------------------------
// --- A r g u m e n t s ---
// -------------------------

#define FOR_EACH_ARGUMENTS_FIELD(VISIT, arg)                         \
  VISIT(Tuple, arguments,        Arguments,       arg)               \
  VISIT(Tuple, keyword_indices,  KeywordIndices,  arg)

class Arguments : public SyntaxTree {
public:
  FOR_EACH_ARGUMENTS_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kArgumentsOffset       = SyntaxTree::kHeaderSize;
  static const uword kKeywordIndicesOffset  = kArgumentsOffset + kPointerSize;
  static const uword kSize                  = kKeywordIndicesOffset + kPointerSize;
};

template <>
class ref_traits<Arguments> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_ARGUMENTS_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Arguments);


// ---------------------------
// --- P a r a m e t e r s ---
// ---------------------------

#define FOR_EACH_PARAMETERS_FIELD(VISIT, arg)                        \
  VISIT(Smi,   position_count, PositionCount, arg)                   \
  VISIT(Tuple, parameters,     Parameters,    arg)

class Parameters : public SyntaxTree {
public:
  FOR_EACH_PARAMETERS_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  inline bool has_keywords();
  inline uword length();

  static const uword kPositionCountOffset = SyntaxTree::kHeaderSize;
  static const uword kParametersOffset    = kPositionCountOffset + kPointerSize;
  static const uword kSize                = kParametersOffset + kPointerSize;
};

template <>
class ref_traits<Parameters> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_PARAMETERS_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Parameters);


// ---------------------------------------------------
// --- I n s t a n t i a t e   E x p r e s s i o n ---
// ---------------------------------------------------

#define FOR_EACH_INSTANTIATE_EXPRESSION_FIELD(VISIT, arg)            \
  VISIT(SyntaxTree, receiver,  Receiver,  arg)                       \
  VISIT(String,     name,      Name,      arg)                       \
  VISIT(Arguments,  arguments, Arguments, arg)                       \
  VISIT(Tuple,      terms,     Terms,     arg)

class InstantiateExpression : public SyntaxTree {
public:
  FOR_EACH_INSTANTIATE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kReceiverOffset  = SyntaxTree::kHeaderSize;
  static const uword kNameOffset      = kReceiverOffset + kPointerSize;
  static const uword kArgumentsOffset = kNameOffset + kPointerSize;
  static const uword kTermsOffset     = kArgumentsOffset + kPointerSize;
  static const uword kSize            = kTermsOffset + kPointerSize;
};

template <>
class ref_traits<InstantiateExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_INSTANTIATE_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(InstantiateExpression);


// ---------------------------------------
// --- R a i s e   E x p r e s s i o n ---
// ---------------------------------------

#define FOR_EACH_RAISE_EXPRESSION_FIELD(VISIT, arg)                  \
  VISIT(String,     name,      Name,      arg)                       \
  VISIT(Arguments,  arguments, Arguments, arg)

class RaiseExpression : public SyntaxTree {
public:
  FOR_EACH_RAISE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset = SyntaxTree::kHeaderSize;
  static const uword kArgumentsOffset = kNameOffset + kPointerSize;
  static const uword kSize = kArgumentsOffset + kPointerSize;
};

template <>
class ref_traits<RaiseExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_RAISE_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(RaiseExpression);


// -------------------------
// --- O n   C l a u s e ---
// -------------------------

#define FOR_EACH_ON_CLAUSE_FIELD(VISIT, arg)                         \
  VISIT(String,           name,   Name,   arg)                       \
  VISIT(LambdaExpression, lambda, Lambda, arg)

class OnClause : public SyntaxTree {
public:
  FOR_EACH_ON_CLAUSE_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset = SyntaxTree::kHeaderSize;
  static const uword kLambdaOffset = kNameOffset + kPointerSize;
  static const uword kSize = kLambdaOffset + kPointerSize;
};

template <>
class ref_traits<OnClause> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_ON_CLAUSE_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(OnClause);


// ---------------------------------------
// --- D o   O n   E x p r e s s i o n ---
// ---------------------------------------

#define FOR_EACH_DO_ON_EXPRESSION_FIELD(VISIT, arg)                  \
  VISIT(SyntaxTree, value,   Value,   arg)                           \
  VISIT(Tuple,      clauses, Clauses, arg)

class DoOnExpression : public SyntaxTree {
public:
  FOR_EACH_DO_ON_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kClausesOffset = kValueOffset + kPointerSize;
  static const uword kSize = kClausesOffset + kPointerSize;
};

template <>
class ref_traits<DoOnExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_DO_ON_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(DoOnExpression);


// -------------------------------------
// --- C a l l   E x p r e s s i o n ---
// -------------------------------------

#define FOR_EACH_CALL_EXPRESSION_FIELD(VISIT, arg)                   \
  VISIT(SyntaxTree, receiver,  Receiver,  arg)                       \
  VISIT(SyntaxTree, function,  Function,  arg)                       \
  VISIT(Arguments,  arguments, Arguments, arg)

class CallExpression : public SyntaxTree {
public:
  FOR_EACH_CALL_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kReceiverOffset = SyntaxTree::kHeaderSize;
  static const uword kFunctionOffset = kReceiverOffset + kPointerSize;
  static const uword kArgumentsOffset = kFunctionOffset + kPointerSize;
  static const uword kSize = kArgumentsOffset + kPointerSize;
};

template <>
class ref_traits<CallExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_CALL_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(CallExpression);


// ---------------------------------------------------
// --- C o n d i t i o n a l   E x p r e s s i o n ---
// ---------------------------------------------------

#define FOR_EACH_CONDITIONAL_EXPRESSION_FIELD(VISIT, arg)            \
  VISIT(SyntaxTree, condition, Condition, arg)                       \
  VISIT(SyntaxTree, then_part, ThenPart,  arg)                       \
  VISIT(SyntaxTree, else_part, ElsePart,  arg)

class ConditionalExpression : public SyntaxTree {
public:
  FOR_EACH_CONDITIONAL_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kConditionOffset = SyntaxTree::kHeaderSize;
  static const uword kThenPartOffset = kConditionOffset + kPointerSize;
  static const uword kElsePartOffset = kThenPartOffset + kPointerSize;
  static const uword kSize = kElsePartOffset + kPointerSize;
};

template <>
class ref_traits<ConditionalExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_CONDITIONAL_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(ConditionalExpression);


// ---------------------------------------
// --- C l a s s   E x p r e s s i o n ---
// ---------------------------------------

#define FOR_EACH_PROTOCOL_EXPRESSION_FIELD(VISIT, arg)               \
  VISIT(String, name,    Name,    arg)                               \
  VISIT(Tuple,  methods, Methods, arg)                               \
  VISIT(Value,  super,   Super,   arg)

class ProtocolExpression : public SyntaxTree {
public:
  FOR_EACH_PROTOCOL_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset = SyntaxTree::kHeaderSize;
  static const uword kMethodsOffset = kNameOffset + kPointerSize;
  static const uword kSuperOffset = kMethodsOffset + kPointerSize;
  static const uword kSize = kSuperOffset + kPointerSize;
};

template <>
class ref_traits<ProtocolExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_PROTOCOL_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
  ref<Protocol> compile(ref<Context> context);
};

DEFINE_REF_CLASS(ProtocolExpression);


// -----------------------------------------
// --- R e t u r n   E x p r e s s i o n ---
// -----------------------------------------

#define FOR_EACH_RETURN_EXPRESSION_FIELD(VISIT, arg)                 \
  VISIT(SyntaxTree, value, Value, arg)

class ReturnExpression : public SyntaxTree {
public:
  FOR_EACH_RETURN_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<ReturnExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_RETURN_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(ReturnExpression);


// ---------------------------------------
// --- Y i e l d   E x p r e s s i o n ---
// ---------------------------------------

#define FOR_EACH_YIELD_EXPRESSION_FIELD(VISIT, arg)                 \
  VISIT(SyntaxTree, value, Value, arg)

class YieldExpression : public SyntaxTree {
public:
  FOR_EACH_YIELD_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<YieldExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_YIELD_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(YieldExpression);


// -----------------------------------------
// --- M e t h o d   E x p r e s s i o n ---
// -----------------------------------------

#define FOR_EACH_METHOD_EXPRESSION_FIELD(VISIT, arg)                 \
  VISIT(Selector,         selector,  Selector, arg)                  \
  VISIT(LambdaExpression, lambda,    Lambda,   arg)                  \
  VISIT(Bool,             is_static, IsStatic, arg)

class MethodExpression : public SyntaxTree {
public:
  FOR_EACH_METHOD_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kSelectorOffset = SyntaxTree::kHeaderSize;
  static const uword kLambdaOffset   = kSelectorOffset + kPointerSize;
  static const uword kIsStaticOffset = kLambdaOffset + kPointerSize;
  static const uword kSize           = kIsStaticOffset + kPointerSize;
};

template <>
class ref_traits<MethodExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_METHOD_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
  ref<Method> compile(ref<Context> context);
};

DEFINE_REF_CLASS(MethodExpression);


// ---------------------------------------------
// --- S e q u e n c e   E x p r e s s i o n ---
// ---------------------------------------------

#define FOR_EACH_SEQUENCE_EXPRESSION_FIELD(VISIT, arg)               \
  VISIT(Tuple, expressions, Expressions, arg)

class SequenceExpression : public SyntaxTree {
public:
  FOR_EACH_SEQUENCE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kExpressionsOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kExpressionsOffset + kPointerSize;
};

template <>
class ref_traits<SequenceExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_SEQUENCE_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(SequenceExpression);


// ---------------------------------------
// --- T u p l e   E x p r e s s i o n ---
// ---------------------------------------

#define FOR_EACH_TUPLE_EXPRESSION_FIELD(VISIT, arg)                  \
  VISIT(Tuple, values, Values, arg)

class TupleExpression : public SyntaxTree {
public:
  FOR_EACH_TUPLE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kValuesOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kValuesOffset + kPointerSize;
};

template <>
class ref_traits<TupleExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_TUPLE_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(TupleExpression);


// -----------------------------------------
// --- G l o b a l   E x p r e s s i o n ---
// -----------------------------------------

#define FOR_EACH_GLOBAL_EXPRESSION_FIELD(VISIT, arg)                 \
  VISIT(String, name, Name, arg)

class GlobalExpression : public SyntaxTree {
public:
  FOR_EACH_GLOBAL_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kNameOffset + kPointerSize;
};

template <>
class ref_traits<GlobalExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_GLOBAL_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(GlobalExpression);


// -------------------
// --- S y m b o l ---
// -------------------

#define FOR_EACH_SYMBOL_FIELD(VISIT, arg)                            \
  VISIT(Value, name, Name, arg)

class Symbol : public SyntaxTree {
public:
  FOR_EACH_SYMBOL_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kNameOffset + kPointerSize;
};

template <>
class ref_traits<Symbol> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_SYMBOL_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Symbol);


// ---------------------------------------
// --- Q u o t e   E x p r e s s i o n ---
// ---------------------------------------

#define FOR_EACH_QUOTE_EXPRESSION_FIELD(VISIT, arg)                  \
  VISIT(SyntaxTree, value,    Value,    arg)                         \
  VISIT(Tuple,      unquotes, Unquotes, arg)

class QuoteExpression : public SyntaxTree {
public:
  FOR_EACH_QUOTE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kUnquotesOffset = kValueOffset + kPointerSize;
  static const uword kSize = kUnquotesOffset + kPointerSize;
};

template <>
class ref_traits<QuoteExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_QUOTE_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(QuoteExpression);


// -------------------------------------------
// --- U n q u o t e   E x p r e s s i o n ---
// -------------------------------------------

class UnquoteExpression : public SyntaxTree {
public:
  DECLARE_FIELD(uword, index);

  static const uword kIndexOffset = SyntaxTree::kHeaderSize;
  static const uword kSize        = kIndexOffset + kPointerSize;
};

template <>
class ref_traits<UnquoteExpression> : public ref_traits<SyntaxTree> {
};

DEFINE_REF_CLASS(UnquoteExpression);

// -----------------------------------------
// --- L a m b d a   E x p r e s s i o n ---
// -----------------------------------------

#define FOR_EACH_LAMBDA_EXPRESSION_FIELD(VISIT, arg)                 \
  VISIT(Parameters, parameters, Parameters, arg)                     \
  VISIT(SyntaxTree, body,       Body,       arg)

class LambdaExpression : public SyntaxTree {
public:
  FOR_EACH_LAMBDA_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kParametersOffset = SyntaxTree::kHeaderSize;
  static const uword kBodyOffset       = kParametersOffset + kPointerSize;
  static const uword kSize             = kBodyOffset + kPointerSize;
};

template <>
class ref_traits<LambdaExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_LAMBDA_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(LambdaExpression);


// -------------------------------------
// --- T a s k   E x p r e s s i o n ---
// -------------------------------------

#define FOR_EACH_TASK_EXPRESSION_FIELD(VISIT, arg)                   \
  VISIT(LambdaExpression, lambda, Lambda, arg)

class TaskExpression : public SyntaxTree {
public:
  FOR_EACH_TASK_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uword kLambdaOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kLambdaOffset + kPointerSize;
};

template <>
class ref_traits<TaskExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_TASK_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(TaskExpression);


// -------------------------------------
// --- T h i s   E x p r e s s i o n ---
// -------------------------------------

#define FOR_EACH_THIS_EXPRESSION_FIELD(VISIT, arg)

class ThisExpression : public SyntaxTree {
public:
  static const uword kSize = SyntaxTree::kHeaderSize;
};


// ---------------------------------------------------
// --- I n t e r p o l a t e   E x p r e s s i o n ---
// ---------------------------------------------------

#define FOR_EACH_INTERPOLATE_EXPRESSION_FIELD(VISIT, arg)            \
  VISIT(Tuple, terms, Terms, arg)

class InterpolateExpression : public SyntaxTree {
public:
  FOR_EACH_INTERPOLATE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kTermsOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kTermsOffset + kPointerSize;
};

template <>
class ref_traits<InterpolateExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_INTERPOLATE_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(InterpolateExpression);


// -------------------------------
// --- B u i l t i n   C a l l ---
// -------------------------------

#define FOR_EACH_BUILTIN_CALL_FIELD(VISIT, arg)

class BuiltinCall : public SyntaxTree {
public:
  DECLARE_FIELD(uword, argc);
  DECLARE_FIELD(uword, index);

  static const uword kArgcOffset = SyntaxTree::kHeaderSize;
  static const uword kIndexOffset = kArgcOffset + kPointerSize;
  static const uword kSize = kIndexOffset + kPointerSize;
};

template <>
class ref_traits<BuiltinCall> : public ref_traits<SyntaxTree> {
public:
  inline uword argc();
  inline uword index();
};

DEFINE_REF_CLASS(BuiltinCall);


// ---------------------------------
// --- E x t e r n a l   C a l l ---
// ---------------------------------

#define FOR_EACH_EXTERNAL_CALL_FIELD(VISIT, arg)                     \
  VISIT(Smi,    argc, Argc, arg)                                     \
  VISIT(String, name, Name, arg)

class ExternalCall : public SyntaxTree {
public:
  FOR_EACH_EXTERNAL_CALL_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kArgcOffset = SyntaxTree::kHeaderSize;
  static const uword kNameOffset = kArgcOffset + kPointerSize;
  static const uword kSize       = kNameOffset + kPointerSize;
};

template <>
class ref_traits<ExternalCall> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_EXTERNAL_CALL_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(ExternalCall);

// ---------------------------------------
// --- L o c a l   D e f i n i t i o n ---
// ---------------------------------------

#define FOR_EACH_LOCAL_DEFINITION_FIELD(VISIT, arg)                  \
  VISIT(Symbol,     symbol, Symbol, arg)                             \
  VISIT(SyntaxTree, value,  Value,  arg)                             \
  VISIT(SyntaxTree, body,   Body,   arg)

class LocalDefinition : public SyntaxTree {
public:
  FOR_EACH_LOCAL_DEFINITION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kSymbolOffset = SyntaxTree::kHeaderSize;
  static const uword kValueOffset = kSymbolOffset + kPointerSize;
  static const uword kBodyOffset = kValueOffset + kPointerSize;
  static const uword kSize = kBodyOffset + kPointerSize;
};

template <>
class ref_traits<LocalDefinition> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_LOCAL_DEFINITION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(LocalDefinition);


// ---------------------------
// --- A s s i g n m e n t ---
// ---------------------------

#define FOR_EACH_ASSIGNMENT_FIELD(VISIT, arg)                        \
  VISIT(Symbol,     symbol, Symbol, arg)                             \
  VISIT(SyntaxTree, value,  Value,  arg)

class Assignment : public SyntaxTree {
public:
  FOR_EACH_ASSIGNMENT_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kSymbolOffset = SyntaxTree::kHeaderSize;
  static const uword kValueOffset = kSymbolOffset + kPointerSize;
  static const uword kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<Assignment> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_ASSIGNMENT_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Assignment);


// ---------------------
// --- V i s i t o r ---
// ---------------------

class QuoteTemplateScope {
public:
  inline QuoteTemplateScope(Visitor &visitor, ref<QuoteTemplate> value);
  inline ~QuoteTemplateScope();
  ref<QuoteTemplate> value() { return value_; }
private:
  Visitor &visitor_;
  ref<QuoteTemplate> value_;
  QuoteTemplateScope *previous_;
};

class Visitor {
public:
  Visitor() : quote_scope_(NULL) { }
  ~Visitor();
  virtual void visit_syntax_tree(ref<SyntaxTree> that);
#define MAKE_VISIT_METHOD(n, NAME, Name, name)                       \
  virtual void visit_##name(ref<Name> that);
FOR_EACH_SYNTAX_TREE_TYPE(MAKE_VISIT_METHOD)
#undef MAKE_VISIT_METHOD
  inline ref<QuoteTemplate> current_quote();
  void set_quote_scope(QuoteTemplateScope *scope) { quote_scope_ = scope; }
  QuoteTemplateScope *quote_scope() { return quote_scope_; }
private:
  QuoteTemplateScope *quote_scope_;
};

}

#endif // _IO_AST
