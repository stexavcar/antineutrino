#ifndef _COMPILER_COMPILER
#define _COMPILER_COMPILER

#include "utils/types.h"

namespace neutrino {

class Compiler {
public:
  static Data *compile(Runtime &runtime, ref<LambdaExpression> tree,
      ref<Context> context);
  static Data *compile(Runtime &runtime, ref<SyntaxTree> tree,
      ref<Context> context);
  static Signal *compile(Runtime &runtime, ref<Lambda> lambda, ref<Method> holder);
};


class VirtualStack {
public:
  VirtualStack() : height_(0), max_height_(0) { }
  uword height() { return height_; }
  uword max_height() { return max_height_; }
  void set_height(uword value);
private:
  uword height_;
  uword max_height_;
};


} // neutrino

#endif // _COMPILER_COMPILER
