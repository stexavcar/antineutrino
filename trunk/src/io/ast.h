#ifndef _IO_AST
#define _IO_AST

#include "heap/values.h"

namespace neutrino {

class Literal : public SyntaxTree {
public:
  inline Value *&value();
  
  static const int kValueOffset = SyntaxTree::kHeaderSize;
  static const int kSize = kValueOffset + kHeaderSize;
};

}

#endif // _IO_AST
