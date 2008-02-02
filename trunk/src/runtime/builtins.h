#ifndef _RUNTIME_BUILTINS
#define _RUNTIME_BUILTINS

#include "runtime/interpreter.h"
#include "utils/types.h"

namespace neutrino {

class Arguments {
public:
  inline Arguments(Runtime &runtime, uword argc, Frame &stack);
  inline Value *self();
  inline Value *operator[](uword index);
  inline Lambda *lambda();
  Runtime &runtime() { return runtime_; }
  uword count() { return count_; }
private:
  Frame &frame() { return frame_; }
  Runtime &runtime_;
  uword count_;
  Frame &frame_;
};

typedef Data *(builtin)(Arguments&);

class Builtins {
public:
  static builtin *get(uword index);
private:

#define DECLARE_BUILTIN(n, layout, name, str) static Data *layout##_##name(Arguments&);
FOR_EACH_BUILTIN_METHOD(DECLARE_BUILTIN)
#undef DECLARE_BUILTIN

#define DECLARE_BUILTIN(n, name, str) static Data *name(Arguments&);
FOR_EACH_BUILTIN_FUNCTION(DECLARE_BUILTIN)
#undef DECLARE_BUILTIN

};

}

#endif // _RUNTIME_BUILTINS
