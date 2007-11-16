#ifndef _COMPILER_COMPILER
#define _COMPILER_COMPILER

#include "utils/types.h"

namespace neutrino {

class Compiler {
public:
  static ref<Lambda> compile(ref<SyntaxTree> tree);
};

} // neutrino

#endif // _COMPILER_COMPILER
