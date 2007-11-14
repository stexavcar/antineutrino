#ifndef _IO_AST
#define _IO_AST

#include "heap/values.h"

namespace neutrino {

class Literal : public SyntaxTree {
public:
  inline Value *&value();
  
  static const uint32_t kValueOffset = SyntaxTree::kHeaderSize;
  static const uint32_t kSize = kValueOffset + kPointerSize;
};

class Invoke : public SyntaxTree {
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

}

#endif // _IO_AST
