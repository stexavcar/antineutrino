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

class LiteralExpression : public SyntaxTree {
public:
  DECLARE_FIELD(Value*, value);
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<LiteralExpression> : public ref_traits<SyntaxTree> {
public:
  inline ref<Value> value();
};

DEFINE_REF_CLASS(LiteralExpression);


// -----------------------------------------
// --- I n v o k e   E x p r e s s i o n ---
// -----------------------------------------

class InvokeExpression : public SyntaxTree {
public:
  DECLARE_FIELD(SyntaxTree*, receiver);
  DECLARE_FIELD(String*, name);
  DECLARE_FIELD(Tuple*, arguments);
  
  static const uint32_t kReceiverOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kNameOffset = kReceiverOffset + kPointerSize;
  static const uint32_t kArgumentsOffset = kNameOffset + kPointerSize;
  static const uint32_t kSize = kArgumentsOffset + kPointerSize;
};


// ---------------------------------------
// --- C l a s s   E x p r e s s i o n ---
// ---------------------------------------

class ClassExpression : public SyntaxTree {
public:
  DECLARE_FIELD(String*, name);
  DECLARE_FIELD(Tuple*, methods);
  DECLARE_FIELD(Value*, super);
  
  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kMethodsOffset = kNameOffset + kPointerSize;
  static const uint32_t kSuperOffset = kMethodsOffset + kPointerSize;
  static const uint32_t kSize = kSuperOffset + kPointerSize;
};

template <>
class ref_traits<ClassExpression> : public ref_traits<SyntaxTree> {
public:
  inline ref<String> name();
  inline ref<Tuple> methods();
  inline ref<Value> super();
  ref<Class> compile();
};

DEFINE_REF_CLASS(ClassExpression);


// -----------------------------------------
// --- R e t u r n   E x p r e s s i o n ---
// -----------------------------------------

class ReturnExpression : public SyntaxTree {
public:
  DECLARE_FIELD(SyntaxTree*, value);
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kValueOffset + kPointerSize;
};

template <>
class ref_traits<ReturnExpression> : public ref_traits<SyntaxTree> {
public:
  inline ref<SyntaxTree> value();
};

DEFINE_REF_CLASS(ReturnExpression);


// -----------------------------------------
// --- M e t h o d   E x p r e s s i o n ---
// -----------------------------------------

class MethodExpression : public SyntaxTree {
public:
  DECLARE_FIELD(String*, name);
  DECLARE_FIELD(SyntaxTree*, body);
  
  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kBodyOffset = kNameOffset + kPointerSize;
  static const uint32_t kSize = kBodyOffset + kPointerSize;
};

template <>
class ref_traits<MethodExpression> : public ref_traits<SyntaxTree> {
public:
  inline ref<String> name();
  inline ref<SyntaxTree> body();
  ref<Method> compile();
};

DEFINE_REF_CLASS(MethodExpression);


// ---------------------------------------------
// --- S e q u e n c e   E x p r e s s i o n ---
// ---------------------------------------------

class SequenceExpression : public SyntaxTree {
public:
  DECLARE_FIELD(Tuple*, expressions);
  
  static const uint32_t kExpressionsOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kExpressionsOffset + kPointerSize;
};

template <>
class ref_traits<SequenceExpression> : public ref_traits<SyntaxTree> {
public:
  inline ref<Tuple> expressions();
};

DEFINE_REF_CLASS(SequenceExpression);


// ---------------------
// --- V i s i t o r ---
// ---------------------

class Visitor {
public:
  virtual ~Visitor();
  virtual void visit_node(ref<SyntaxTree> that);
  virtual void visit_literal_expression(ref<LiteralExpression> that);
  virtual void visit_return_expression(ref<ReturnExpression> that);
  virtual void visit_sequence_expression(ref<SequenceExpression> that);
};


}

#endif // _IO_AST
