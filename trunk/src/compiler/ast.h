#ifndef _IO_AST
#define _IO_AST

#include "heap/values.h"

namespace neutrino {

// -----------------------------
// --- S y n t a x   T r e e ---
// -----------------------------

class SyntaxTree : public Object {
public:
  inline Value *&at(uint32_t index);
  
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
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kValueOffset + kPointerSize;
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
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kUnquotesOffset = kValueOffset + kPointerSize;
  static const uint32_t kSize = kUnquotesOffset + kPointerSize;
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
  VISIT(String,     name,      Name,      arg)                       \
  VISIT(Tuple,      arguments, Arguments, arg)

class InvokeExpression : public SyntaxTree {
public:
  FOR_EACH_INVOKE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kReceiverOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kNameOffset = kReceiverOffset + kPointerSize;
  static const uint32_t kArgumentsOffset = kNameOffset + kPointerSize;
  static const uint32_t kSize = kArgumentsOffset + kPointerSize;
};

template <>
class ref_traits<InvokeExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_INVOKE_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(InvokeExpression);


// ---------------------------------------------------
// --- I n s t a n t i a t e   E x p r e s s i o n ---
// ---------------------------------------------------

#define FOR_EACH_INSTANTIATE_EXPRESSION_FIELD(VISIT, arg)            \
  VISIT(SyntaxTree, receiver,  Receiver,  arg)                       \
  VISIT(String,     name,      Name,      arg)                       \
  VISIT(Tuple,      arguments, Arguments, arg)                       \
  VISIT(Tuple,      terms,     Terms,     arg)

class InstantiateExpression : public SyntaxTree {
public:
  FOR_EACH_INSTANTIATE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kReceiverOffset  = SyntaxTree::kHeaderSize;
  static const uint32_t kNameOffset      = kReceiverOffset + kPointerSize;
  static const uint32_t kArgumentsOffset = kNameOffset + kPointerSize;
  static const uint32_t kTermsOffset     = kArgumentsOffset + kPointerSize;
  static const uint32_t kSize            = kTermsOffset + kPointerSize;
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
  VISIT(Tuple,      arguments, Arguments, arg)

class RaiseExpression : public SyntaxTree {
public:
  FOR_EACH_RAISE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kArgumentsOffset = kNameOffset + kPointerSize;
  static const uint32_t kSize = kArgumentsOffset + kPointerSize;
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

  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kLambdaOffset = kNameOffset + kPointerSize;
  static const uint32_t kSize = kLambdaOffset + kPointerSize;
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
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kClausesOffset = kValueOffset + kPointerSize;
  static const uint32_t kSize = kClausesOffset + kPointerSize;
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
  VISIT(Tuple,      arguments, Arguments, arg)

class CallExpression : public SyntaxTree {
public:
  FOR_EACH_CALL_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kReceiverOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kFunctionOffset = kReceiverOffset + kPointerSize;
  static const uint32_t kArgumentsOffset = kFunctionOffset + kPointerSize;
  static const uint32_t kSize = kArgumentsOffset + kPointerSize;
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
  
  static const uint32_t kConditionOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kThenPartOffset = kConditionOffset + kPointerSize;
  static const uint32_t kElsePartOffset = kThenPartOffset + kPointerSize;
  static const uint32_t kSize = kElsePartOffset + kPointerSize;
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

#define FOR_EACH_CLASS_EXPRESSION_FIELD(VISIT, arg)                  \
  VISIT(String, name,    Name,    arg)                               \
  VISIT(Tuple,  methods, Methods, arg)                               \
  VISIT(Value,  super,   Super,   arg)

class ClassExpression : public SyntaxTree {
public:
  FOR_EACH_CLASS_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kMethodsOffset = kNameOffset + kPointerSize;
  static const uint32_t kSuperOffset = kMethodsOffset + kPointerSize;
  static const uint32_t kSize = kSuperOffset + kPointerSize;
};

template <>
class ref_traits<ClassExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_CLASS_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
  ref<Layout> compile();
};

DEFINE_REF_CLASS(ClassExpression);


// -----------------------------------------
// --- R e t u r n   E x p r e s s i o n ---
// -----------------------------------------

#define FOR_EACH_RETURN_EXPRESSION_FIELD(VISIT, arg)                 \
  VISIT(SyntaxTree, value, Value, arg)

class ReturnExpression : public SyntaxTree {
public:
  FOR_EACH_RETURN_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<ReturnExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_RETURN_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(ReturnExpression);


// -----------------------------------------
// --- M e t h o d   E x p r e s s i o n ---
// -----------------------------------------

#define FOR_EACH_METHOD_EXPRESSION_FIELD(VISIT, arg)                 \
  VISIT(String,           name,   Name,   arg)                       \
  VISIT(LambdaExpression, lambda, Lambda, arg)

class MethodExpression : public SyntaxTree {
public:
  FOR_EACH_METHOD_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kLambdaOffset = kNameOffset + kPointerSize;
  static const uint32_t kSize = kLambdaOffset + kPointerSize;
};

template <>
class ref_traits<MethodExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_METHOD_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
  ref<Method> compile();
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
  
  static const uint32_t kExpressionsOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kExpressionsOffset + kPointerSize;
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

  static const uint32_t kValuesOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kValuesOffset + kPointerSize;
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
  
  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kNameOffset + kPointerSize;
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
  
  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kNameOffset + kPointerSize;
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
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kUnquotesOffset = kValueOffset + kPointerSize;
  static const uint32_t kSize = kUnquotesOffset + kPointerSize;
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
  DECLARE_FIELD(uint32_t, index);
  
  static const uint32_t kIndexOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize        = kIndexOffset + kPointerSize;
};

template <>
class ref_traits<UnquoteExpression> : public ref_traits<SyntaxTree> {
};

DEFINE_REF_CLASS(UnquoteExpression);

// -----------------------------------------
// --- L a m b d a   E x p r e s s i o n ---
// -----------------------------------------

#define FOR_EACH_LAMBDA_EXPRESSION_FIELD(VISIT, arg)                 \
  VISIT(Tuple,      params, Params, arg)                             \
  VISIT(SyntaxTree, body,   Body,   arg)

class LambdaExpression : public SyntaxTree {
public:
  FOR_EACH_LAMBDA_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kParamsOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kBodyOffset = kParamsOffset + kPointerSize;
  static const uint32_t kSize = kBodyOffset + kPointerSize;
};

template <>
class ref_traits<LambdaExpression> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_LAMBDA_EXPRESSION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(LambdaExpression);

// -------------------------------------
// --- T h i s   E x p r e s s i o n ---
// -------------------------------------

#define FOR_EACH_THIS_EXPRESSION_FIELD(VISIT, arg)

class ThisExpression : public SyntaxTree {
public:
  static const uint32_t kSize = SyntaxTree::kHeaderSize;
};


// ---------------------------------------------------
// --- I n t e r p o l a t e   E x p r e s s i o n ---
// ---------------------------------------------------

#define FOR_EACH_INTERPOLATE_EXPRESSION_FIELD(VISIT, arg)            \
  VISIT(Tuple, terms, Terms, arg)

class InterpolateExpression : public SyntaxTree {
public:
  FOR_EACH_INTERPOLATE_EXPRESSION_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uint32_t kTermsOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kTermsOffset + kPointerSize;
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
  DECLARE_FIELD(uint32_t, argc);
  DECLARE_FIELD(uint32_t, index);

  static const uint32_t kArgcOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kIndexOffset = kArgcOffset + kPointerSize;
  static const uint32_t kSize = kIndexOffset + kPointerSize;
};

template <>
class ref_traits<BuiltinCall> : public ref_traits<SyntaxTree> {
public:
  inline uint32_t argc();
  inline uint32_t index();
};

DEFINE_REF_CLASS(BuiltinCall);


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
  
  static const uint32_t kSymbolOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kValueOffset = kSymbolOffset + kPointerSize;
  static const uint32_t kBodyOffset = kValueOffset + kPointerSize;
  static const uint32_t kSize = kBodyOffset + kPointerSize;
};

template <>
class ref_traits<LocalDefinition> : public ref_traits<SyntaxTree> {
public:
  FOR_EACH_LOCAL_DEFINITION_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(LocalDefinition);


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
