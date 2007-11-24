#ifndef _COMPILER_COMPILER
#define _COMPILER_COMPILER

#include "utils/types.h"

namespace neutrino {

class Compiler {
public:
  static ref<Lambda> compile(ref<MethodExpression> tree);
  static void compile(ref<Lambda> lambda);
};

} // neutrino

#endif // _COMPILER_COMPILER
