#ifndef _IO_AST
#define _IO_AST

#include "heap/values.h"

namespace neutrino {

class LiteralExpression : public SyntaxTree {
public:
  inline Value *&value();
  
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

class ClassExpression : public SyntaxTree {
public:
  inline String *&name();
  inline void set_name(String *value);
  inline Tuple *&methods();
  inline void set_methods(Tuple *value);
  
  static const uint32_t kNameOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kMethodsOffset = kNameOffset + kPointerSize;
  static const uint32_t kSize = kMethodsOffset + kPointerSize;
};

template <>
class ref_traits<ClassExpression> {
public:
  ref<Class> evaluate();
};

DEFINE_REF_CLASS(ClassExpression);

class ReturnExpression : public SyntaxTree {
public:
  inline SyntaxTree *&value();
  inline void set_value(SyntaxTree *value);
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kValueOffset + kPointerSize;
};

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

}

#endif // _IO_AST
