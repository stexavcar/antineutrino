#ifndef _RUNTIME_BUILTINS
#define _RUNTIME_BUILTINS

#include "runtime/interpreter.h"
#include "utils/types.h"

namespace neutrino {

class Arguments {
public:
  inline Arguments(Runtime &runtime, uint32_t argc, Frame &stack);
  inline Value *self();
  inline Value *operator[](uint32_t index);
  Runtime &runtime() { return runtime_; }
  uint32_t count() { return count_; }
private:
  Frame &frame() { return frame_; }
  Runtime &runtime_;
  uint32_t count_;
  Frame &frame_;
};

typedef Data *(builtin)(Arguments&);

class Builtins {
public:
  static builtin *get(uint32_t index);
private:

#define DECLARE_BUILTIN(n, chlass, name, str) static Data *chlass##_##name(Arguments&);
FOR_EACH_BUILTIN_METHOD(DECLARE_BUILTIN)
#undef DECLARE_BUILTIN

#define DECLARE_BUILTIN(n, name, str) static Data *name(Arguments&);
FOR_EACH_BUILTIN_FUNCTION(DECLARE_BUILTIN)
#undef DECLARE_BUILTIN

};

}

#endif // _RUNTIME_BUILTINS
