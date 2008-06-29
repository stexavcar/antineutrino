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
  Signal *accept(Visitor &visitor);
  Signal *traverse(Visitor &visitor);
};

DEFINE_REF_CLASS(SyntaxTree);


// -------------------------------------------
// --- L i t e r a l   E x p r e s s i o n ---
// -------------------------------------------

#define eLiteralExpressionFields(VISIT, arg)                \
  VISIT(Value, value, Value, arg)

class LiteralExpression : public SyntaxTree {
public:
  eLiteralExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<LiteralExpression> : public ref_traits<SyntaxTree> {
public:
  eLiteralExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(LiteralExpression);

// -----------------------------------------
// --- I n v o k e   E x p r e s s i o n ---
// -----------------------------------------

#define eInvokeExpressionFields(VISIT, arg)                 \
  VISIT(SyntaxTree, receiver,  Receiver,  arg)                       \
  VISIT(Selector,   selector,  Selector,  arg)                       \
  VISIT(Arguments,  arguments, Arguments, arg)

class InvokeExpression : public SyntaxTree {
public:
  eInvokeExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kReceiverOffset  = SyntaxTree::kHeaderSize;
  static const uword kSelectorOffset  = kReceiverOffset + kPointerSize;
  static const uword kArgumentsOffset = kSelectorOffset + kPointerSize;
  static const uword kSize            = kArgumentsOffset + kPointerSize;
};

template <>
class ref_traits<InvokeExpression> : public ref_traits<SyntaxTree> {
public:
  eInvokeExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(InvokeExpression);


// -------------------------
// --- A r g u m e n t s ---
// -------------------------

#define eArgumentsFields(VISIT, arg)                         \
  VISIT(Tuple, arguments,        Arguments,       arg)               \
  VISIT(Tuple, keyword_indices,  KeywordIndices,  arg)

class Arguments : public SyntaxTree {
public:
  eArgumentsFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kArgumentsOffset       = SyntaxTree::kHeaderSize;
  static const uword kKeywordIndicesOffset  = kArgumentsOffset + kPointerSize;
  static const uword kSize                  = kKeywordIndicesOffset + kPointerSize;
};

template <>
class ref_traits<Arguments> : public ref_traits<SyntaxTree> {
public:
  eArgumentsFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Arguments);


// ---------------------------
// --- P a r a m e t e r s ---
// ---------------------------

#define eParametersFields(VISIT, arg)                        \
  VISIT(Smi,   position_count, PositionCount, arg)                   \
  VISIT(Tuple, parameters,     Parameters,    arg)

class Parameters : public SyntaxTree {
public:
  eParametersFields(DECLARE_OBJECT_FIELD, 0)

  inline bool has_keywords();
  inline uword length();

  static const uword kPositionCountOffset = SyntaxTree::kHeaderSize;
  static const uword kParametersOffset    = kPositionCountOffset + kPointerSize;
  static const uword kSize                = kParametersOffset + kPointerSize;
};

template <>
class ref_traits<Parameters> : public ref_traits<SyntaxTree> {
public:
  eParametersFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Parameters);


// ---------------------------------------------------
// --- I n s t a n t i a t e   E x p r e s s i o n ---
// ---------------------------------------------------

#define eInstantiateExpressionFields(VISIT, arg)            \
  VISIT(SyntaxTree, receiver,  Receiver,  arg)                       \
  VISIT(String,     name,      Name,      arg)                       \
  VISIT(Arguments,  arguments, Arguments, arg)                       \
  VISIT(Tuple,      terms,     Terms,     arg)

class InstantiateExpression : public SyntaxTree {
public:
  eInstantiateExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kReceiverOffset  = SyntaxTree::kHeaderSize;
  static const uword kNameOffset      = kReceiverOffset + kPointerSize;
  static const uword kArgumentsOffset = kNameOffset + kPointerSize;
  static const uword kTermsOffset     = kArgumentsOffset + kPointerSize;
  static const uword kSize            = kTermsOffset + kPointerSize;
};

template <>
class ref_traits<InstantiateExpression> : public ref_traits<SyntaxTree> {
public:
  eInstantiateExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(InstantiateExpression);


// ---------------------------------------
// --- R a i s e   E x p r e s s i o n ---
// ---------------------------------------

#define eRaiseExpressionFields(VISIT, arg)                  \
  VISIT(String,     name,      Name,      arg)                       \
  VISIT(Arguments,  arguments, Arguments, arg)

class RaiseExpression : public SyntaxTree {
public:
  eRaiseExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset = SyntaxTree::kHeaderSize;
  static const uword kArgumentsOffset = kNameOffset + kPointerSize;
  static const uword kSize = kArgumentsOffset + kPointerSize;
};

template <>
class ref_traits<RaiseExpression> : public ref_traits<SyntaxTree> {
public:
  eRaiseExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(RaiseExpression);


// -------------------------
// --- O n   C l a u s e ---
// -------------------------

#define eOnClauseFields(VISIT, arg)                         \
  VISIT(String,           name,   Name,   arg)                       \
  VISIT(LambdaExpression, lambda, Lambda, arg)

class OnClause : public SyntaxTree {
public:
  eOnClauseFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset = SyntaxTree::kHeaderSize;
  static const uword kLambdaOffset = kNameOffset + kPointerSize;
  static const uword kSize = kLambdaOffset + kPointerSize;
};

template <>
class ref_traits<OnClause> : public ref_traits<SyntaxTree> {
public:
  eOnClauseFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(OnClause);


// ---------------------------------------
// --- D o   O n   E x p r e s s i o n ---
// ---------------------------------------

#define eDoOnExpressionFields(VISIT, arg)                  \
  VISIT(SyntaxTree, value,   Value,   arg)                           \
  VISIT(Tuple,      clauses, Clauses, arg)

class DoOnExpression : public SyntaxTree {
public:
  eDoOnExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kClausesOffset = kValueOffset + kPointerSize;
  static const uword kSize = kClausesOffset + kPointerSize;
};

template <>
class ref_traits<DoOnExpression> : public ref_traits<SyntaxTree> {
public:
  eDoOnExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(DoOnExpression);


// -------------------------------------
// --- C a l l   E x p r e s s i o n ---
// -------------------------------------

#define eCallExpressionFields(VISIT, arg)                   \
  VISIT(SyntaxTree, receiver,  Receiver,  arg)                       \
  VISIT(SyntaxTree, function,  Function,  arg)                       \
  VISIT(Arguments,  arguments, Arguments, arg)

class CallExpression : public SyntaxTree {
public:
  eCallExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kReceiverOffset = SyntaxTree::kHeaderSize;
  static const uword kFunctionOffset = kReceiverOffset + kPointerSize;
  static const uword kArgumentsOffset = kFunctionOffset + kPointerSize;
  static const uword kSize = kArgumentsOffset + kPointerSize;
};

template <>
class ref_traits<CallExpression> : public ref_traits<SyntaxTree> {
public:
  eCallExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(CallExpression);


// ---------------------------------------------------
// --- C o n d i t i o n a l   E x p r e s s i o n ---
// ---------------------------------------------------

#define eConditionalExpressionFields(VISIT, arg)            \
  VISIT(SyntaxTree, condition, Condition, arg)                       \
  VISIT(SyntaxTree, then_part, ThenPart,  arg)                       \
  VISIT(SyntaxTree, else_part, ElsePart,  arg)

class ConditionalExpression : public SyntaxTree {
public:
  eConditionalExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kConditionOffset = SyntaxTree::kHeaderSize;
  static const uword kThenPartOffset = kConditionOffset + kPointerSize;
  static const uword kElsePartOffset = kThenPartOffset + kPointerSize;
  static const uword kSize = kElsePartOffset + kPointerSize;
};

template <>
class ref_traits<ConditionalExpression> : public ref_traits<SyntaxTree> {
public:
  eConditionalExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(ConditionalExpression);


// ---------------------------------------
// --- W h i l e   E x p r e s s i o n ---
// ---------------------------------------

#define eWhileExpressionFields(VISIT, arg)                  \
  VISIT(SyntaxTree, condition, Condition,  arg)                      \
  VISIT(SyntaxTree, body,      Body,  arg)

class WhileExpression : public SyntaxTree {
public:
  eWhileExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kConditionOffset = SyntaxTree::kHeaderSize;
  static const uword kBodyOffset      = kConditionOffset + kPointerSize;
  static const uword kSize            = kBodyOffset + kPointerSize;
};

template <>
class ref_traits<WhileExpression> : public ref_traits<SyntaxTree> {
public:
  eWhileExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(WhileExpression);


// ---------------------------------------
// --- C l a s s   E x p r e s s i o n ---
// ---------------------------------------

#define eProtocolExpressionFields(VISIT, arg)               \
  VISIT(String, name,    Name,    arg)                               \
  VISIT(Tuple,  methods, Methods, arg)                               \
  VISIT(Value,  super,   Super,   arg)

class ProtocolExpression : public SyntaxTree {
public:
  eProtocolExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset = SyntaxTree::kHeaderSize;
  static const uword kMethodsOffset = kNameOffset + kPointerSize;
  static const uword kSuperOffset = kMethodsOffset + kPointerSize;
  static const uword kSize = kSuperOffset + kPointerSize;
};

template <>
class ref_traits<ProtocolExpression> : public ref_traits<SyntaxTree> {
public:
  eProtocolExpressionFields(DECLARE_REF_FIELD, 0)
  Option<Protocol> compile(Runtime &runtime, ref<Context> context);
};

DEFINE_REF_CLASS(ProtocolExpression);


// -----------------------------------------
// --- R e t u r n   E x p r e s s i o n ---
// -----------------------------------------

#define eReturnExpressionFields(VISIT, arg)                 \
  VISIT(SyntaxTree, value, Value, arg)

class ReturnExpression : public SyntaxTree {
public:
  eReturnExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<ReturnExpression> : public ref_traits<SyntaxTree> {
public:
  eReturnExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(ReturnExpression);


// ---------------------------------------
// --- Y i e l d   E x p r e s s i o n ---
// ---------------------------------------

#define eYieldExpressionFields(VISIT, arg)                 \
  VISIT(SyntaxTree, value, Value, arg)

class YieldExpression : public SyntaxTree {
public:
  eYieldExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<YieldExpression> : public ref_traits<SyntaxTree> {
public:
  eYieldExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(YieldExpression);


// -----------------------------------------
// --- M e t h o d   E x p r e s s i o n ---
// -----------------------------------------

#define eMethodExpressionFields(VISIT, arg)                 \
  VISIT(Selector,         selector,  Selector, arg)                  \
  VISIT(LambdaExpression, lambda,    Lambda,   arg)                  \
  VISIT(Bool,             is_static, IsStatic, arg)

class MethodExpression : public SyntaxTree {
public:
  eMethodExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kSelectorOffset = SyntaxTree::kHeaderSize;
  static const uword kLambdaOffset   = kSelectorOffset + kPointerSize;
  static const uword kIsStaticOffset = kLambdaOffset + kPointerSize;
  static const uword kSize           = kIsStaticOffset + kPointerSize;
};

template <>
class ref_traits<MethodExpression> : public ref_traits<SyntaxTree> {
public:
  eMethodExpressionFields(DECLARE_REF_FIELD, 0)
  Option<Method> compile(Runtime &runtime, ref<Context> context);
};

DEFINE_REF_CLASS(MethodExpression);


// ---------------------------------------------
// --- S e q u e n c e   E x p r e s s i o n ---
// ---------------------------------------------

#define eSequenceExpressionFields(VISIT, arg)               \
  VISIT(Tuple, expressions, Expressions, arg)

class SequenceExpression : public SyntaxTree {
public:
  eSequenceExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kExpressionsOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kExpressionsOffset + kPointerSize;
};

template <>
class ref_traits<SequenceExpression> : public ref_traits<SyntaxTree> {
public:
  eSequenceExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(SequenceExpression);


// ---------------------------------------
// --- T u p l e   E x p r e s s i o n ---
// ---------------------------------------

#define eTupleExpressionFields(VISIT, arg)                  \
  VISIT(Tuple, values, Values, arg)

class TupleExpression : public SyntaxTree {
public:
  eTupleExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kValuesOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kValuesOffset + kPointerSize;
};

template <>
class ref_traits<TupleExpression> : public ref_traits<SyntaxTree> {
public:
  eTupleExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(TupleExpression);


// -------------------------------------
// --- G l o b a l   V a r i a b l e ---
// -------------------------------------

#define eGlobalVariableFields(VISIT, arg)                            \
  VISIT(String, name, Name, arg)

class GlobalVariable : public SyntaxTree {
public:
  eGlobalVariableFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kNameOffset + kPointerSize;
};

template <>
class ref_traits<GlobalVariable> : public ref_traits<SyntaxTree> {
public:
  eGlobalVariableFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(GlobalVariable);


// -----------------------------------
// --- L o c a l   V a r i a b l e ---
// -----------------------------------

#define eLocalVariableFields(VISIT, arg)                             \
  VISIT(Symbol, symbol, Symbol, arg)

class LocalVariable : public SyntaxTree {
public:
  eLocalVariableFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kSymbolOffset = SyntaxTree::kHeaderSize;
  static const uword kSize         = kSymbolOffset + kPointerSize;
};

template <>
class ref_traits<LocalVariable> : public ref_traits<SyntaxTree> {
public:
  eLocalVariableFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(LocalVariable);


// -------------------
// --- S y m b o l ---
// -------------------

#define eSymbolFields(VISIT, arg)                                    \
  VISIT(Value,     name, Name, arg)                                  \
  VISIT(Immediate, data, Data, arg)

class Symbol : public Object {
public:
  eSymbolFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset = SyntaxTree::kHeaderSize;
  static const uword kDataOffset = kNameOffset + kPointerSize;
  static const uword kSize       = kDataOffset + kPointerSize;
};

template <>
class ref_traits<Symbol> : public ref_traits<Object> {
public:
  eSymbolFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Symbol);


// -----------------------------------------
// --- L a m b d a   E x p r e s s i o n ---
// -----------------------------------------

#define eLambdaExpressionFields(VISIT, arg)                          \
  VISIT(Parameters, parameters, Parameters, arg)                     \
  VISIT(SyntaxTree, body,       Body,       arg)                     \
  VISIT(Bool,       is_local,   IsLocal,    arg)

class LambdaExpression : public SyntaxTree {
public:
  eLambdaExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kParametersOffset = SyntaxTree::kHeaderSize;
  static const uword kBodyOffset       = kParametersOffset + kPointerSize;
  static const uword kIsLocalOffset    = kBodyOffset + kPointerSize;
  static const uword kSize             = kIsLocalOffset + kPointerSize;
};

template <>
class ref_traits<LambdaExpression> : public ref_traits<SyntaxTree> {
public:
  eLambdaExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(LambdaExpression);


// -------------------------------------
// --- T a s k   E x p r e s s i o n ---
// -------------------------------------

#define eTaskExpressionFields(VISIT, arg)                   \
  VISIT(LambdaExpression, lambda, Lambda, arg)

class TaskExpression : public SyntaxTree {
public:
  eTaskExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kLambdaOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kLambdaOffset + kPointerSize;
};

template <>
class ref_traits<TaskExpression> : public ref_traits<SyntaxTree> {
public:
  eTaskExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(TaskExpression);


// -------------------------------------
// --- T h i s   E x p r e s s i o n ---
// -------------------------------------

#define eThisExpressionFields(VISIT, arg)

class ThisExpression : public SyntaxTree {
public:
  static const uword kSize = SyntaxTree::kHeaderSize;
};


// ---------------------------------------
// --- S u p e r   E x p r e s s i o n ---
// ---------------------------------------

#define eSuperExpressionFields(VISIT, arg)                  \
  VISIT(SyntaxTree, value, Value, arg)

class SuperExpression : public SyntaxTree {
public:
  eSuperExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = SyntaxTree::kHeaderSize;
  static const uword kSize        = kValueOffset + kPointerSize;
};

template <>
class ref_traits<SuperExpression> : public ref_traits<SyntaxTree> {
public:
  eSuperExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(SuperExpression);


// ---------------------------------------------------
// --- I n t e r p o l a t e   E x p r e s s i o n ---
// ---------------------------------------------------

#define eInterpolateExpressionFields(VISIT, arg)            \
  VISIT(Tuple, terms, Terms, arg)

class InterpolateExpression : public SyntaxTree {
public:
  eInterpolateExpressionFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kTermsOffset = SyntaxTree::kHeaderSize;
  static const uword kSize = kTermsOffset + kPointerSize;
};

template <>
class ref_traits<InterpolateExpression> : public ref_traits<SyntaxTree> {
public:
  eInterpolateExpressionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(InterpolateExpression);


// -------------------------------
// --- B u i l t i n   C a l l ---
// -------------------------------

#define eBuiltinCallFields(VISIT, arg)

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


// ---------------------------------------
// --- L o c a l   D e f i n i t i o n ---
// ---------------------------------------

#define eLocalDefinitionFields(VISIT, arg)                  \
  VISIT(Symbol,     symbol, Symbol, arg)                             \
  VISIT(SyntaxTree, value,  Value,  arg)                             \
  VISIT(SyntaxTree, body,   Body,   arg)                             \
  VISIT(Smi,        type,   Type,   arg)

class LocalDefinition : public SyntaxTree {
public:
  eLocalDefinitionFields(DECLARE_OBJECT_FIELD, 0)

  enum Type {
    ldNone = 0,
    ldDef = 1,
    ldVar = 2,
    ldRec = 3,
    ldLoc = 4
  };

  static const uword kSymbolOffset = SyntaxTree::kHeaderSize;
  static const uword kValueOffset  = kSymbolOffset + kPointerSize;
  static const uword kBodyOffset   = kValueOffset + kPointerSize;
  static const uword kTypeOffset   = kBodyOffset + kPointerSize;
  static const uword kSize         = kTypeOffset + kPointerSize;
};

template <>
class ref_traits<LocalDefinition> : public ref_traits<SyntaxTree> {
public:
  eLocalDefinitionFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(LocalDefinition);


// ---------------------------
// --- A s s i g n m e n t ---
// ---------------------------

#define eAssignmentFields(VISIT, arg)                        \
  VISIT(Symbol,     symbol, Symbol, arg)                             \
  VISIT(SyntaxTree, value,  Value,  arg)

class Assignment : public SyntaxTree {
public:
  eAssignmentFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kSymbolOffset = SyntaxTree::kHeaderSize;
  static const uword kValueOffset = kSymbolOffset + kPointerSize;
  static const uword kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<Assignment> : public ref_traits<SyntaxTree> {
public:
  eAssignmentFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Assignment);


// ---------------------
// --- V i s i t o r ---
// ---------------------

class Visitor {
public:
  Visitor(RefManager &refs, Visitor *enclosing)
      : refs_(refs)
      , scope_(enclosing ? enclosing->scope_ : NULL) { }
  ~Visitor();
  virtual Signal *visit_symbol(ref<Symbol> that);
  virtual Signal *visit_syntax_tree(ref<SyntaxTree> that);
#define MAKE_VISIT_METHOD(n, Name, name)                             \
  virtual Signal *visit_##name(ref<Name> that);
eSyntaxTreeTypes(MAKE_VISIT_METHOD)
#undef MAKE_VISIT_METHOD
  RefManager &refs() { return refs_; }
  Scope &scope() { return *scope_; }

private:
  friend class Scope;
  RefManager &refs_;
  Scope *scope_;
};

}

#endif // _IO_AST
