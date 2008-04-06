#ifndef _COMPILER_COMPILER
#define _COMPILER_COMPILER

#include "utils/types.h"

namespace neutrino {

class Compiler {
public:
  static ref<Lambda> compile(Runtime &runtime, ref<LambdaExpression> tree,
      ref<Context> context);
  static ref<Lambda> compile(Runtime &runtime, ref<SyntaxTree> tree,
      ref<Context> context);
  static void compile(Runtime &runtime, ref<Lambda> lambda, ref<Method> holder);
};

} // neutrino

#endif // _COMPILER_COMPILER
