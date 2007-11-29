#ifndef _RUNTIME_BUILTINS
#define _RUNTIME_BUILTINS

#include "runtime/interpreter.h"
#include "utils/types.h"

namespace neutrino {

class Arguments {
public:
  inline Arguments(Runtime &runtime, uint32_t argc, OldStack &stack);
  inline ref<Value> self();
  inline ref<Value> operator[](uint32_t index);
  Runtime &runtime() { return runtime_; }
  uint32_t count() { return count_; }
private:
  OldStack &stack() { return stack_; }
  Runtime &runtime_;
  uint32_t count_;
  OldStack &stack_;
};

typedef Value *(Builtin)(Arguments&);

class Builtins {
public:
  static Builtin *get(uint32_t index);
private:

#define DECLARE_BUILTIN(n, chlass, name, str) static Value *chlass##_##name(Arguments&);
FOR_EACH_BUILTIN_METHOD(DECLARE_BUILTIN)
#undef DECLARE_BUILTIN

#define DECLARE_BUILTIN(n, name, str) static Value *name(Arguments&);
FOR_EACH_BUILTIN_FUNCTION(DECLARE_BUILTIN)
#undef DECLARE_BUILTIN

};

}

#endif // _RUNTIME_BUILTINS
