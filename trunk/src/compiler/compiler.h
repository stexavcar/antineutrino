#ifndef _COMPILER_COMPILER
#define _COMPILER_COMPILER

#include "utils/types.h"

namespace neutrino {

class Compiler {
public:
  static ref<Lambda> compile(ref<LambdaExpression> tree, ref<Context> context);
  static ref<Lambda> compile(ref<SyntaxTree> tree, ref<Context> context);
  static void compile(ref<Lambda> lambda);
};

} // neutrino

#endif // _COMPILER_COMPILER
