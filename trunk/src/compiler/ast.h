#ifndef _IO_AST
#define _IO_AST

#include "heap/values.h"

namespace neutrino {

class LiteralExpression : public SyntaxTree {
public:
  DEFINE_FIELD(Value*, value)
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kValueOffset + kPointerSize;
};

class InvokeExpression : public SyntaxTree {
public:
  inline SyntaxTree *&receiver();
  inline void set_receiver(SyntaxTree *value);
  inline String *&name();
  inline void set_name(String *value);
  inline Tuple *&arguments();
  inline void set_arguments(Tuple *value);
  
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
  inline String *&name();
  inline void set_name(String *value);
  inline Tuple *&methods();
  inline void set_methods(Tuple *value);
  inline Class *&super();
  inline void set_super(Class *value);
  
  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kMethodsOffset = kNameOffset + kPointerSize;
  static const uint32_t kSuperOffset = kMethodsOffset + kPointerSize;
  static const uint32_t kSize = kSuperOffset + kPointerSize;
};

template <>
class ref_traits<ClassExpression> {
public:
  inline ref<String> name();
  inline ref<Tuple> methods();
  inline ref<Class> super();
  ref<Class> compile();
};

DEFINE_REF_CLASS(ClassExpression);


// -----------------------------------------
// --- R e t u r n   E x p r e s s i o n ---
// -----------------------------------------

class ReturnExpression : public SyntaxTree {
public:
  inline SyntaxTree *&value();
  inline void set_value(SyntaxTree *value);
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kValueOffset + kPointerSize;
};


// -----------------------------------------
// --- M e t h o d   E x p r e s s i o n ---
// -----------------------------------------

class MethodExpression : public SyntaxTree {
public:
  inline String *&name();
  inline void set_name(String *value);
  inline SyntaxTree *&body();
  inline void set_body(SyntaxTree *value);
  
  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kBodyOffset = kNameOffset + kPointerSize;
  static const uint32_t kSize = kBodyOffset + kPointerSize;
};

template <>
class ref_traits<MethodExpression> {
public:
  inline ref<String> name();
  inline ref<SyntaxTree> body();
  ref<Method> compile();
};

DEFINE_REF_CLASS(MethodExpression);

}

#endif // _IO_AST
