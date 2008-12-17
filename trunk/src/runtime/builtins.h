#ifndef _RUNTIME_BUILTINS
#define _RUNTIME_BUILTINS

#include "runtime/interpreter.h"
#include "utils/checks.h"
#include "utils/types.h"

namespace neutrino {

class BuiltinArguments {
public:
  inline BuiltinArguments(Runtime &runtime, uword argc, StackState &stack);
  inline Value *self();
  inline Value *operator[](uword index);
  inline Lambda *lambda();
  Runtime &runtime() { return runtime_; }
  uword count() { return count_; }
private:
  StackState &frame() { return frame_; }
  Runtime &runtime_;
  uword count_;
  StackState &frame_;
};

class MessageArguments {
public:
  inline MessageArguments(Runtime &runtime, uword count, StackState &stack);
  inline Value *operator[](uword index);
  Runtime &runtime() { return runtime_; }
private:
  StackState &frame() { return frame_; }
  uword count() { return count_; }
  Runtime &runtime_;
  uword count_;
  StackState &frame_;
};

typedef Data *(builtin)(BuiltinArguments&);

class Builtins {
public:
  static builtin *get(uword index);
private:

#define DECLARE_BUILTIN(n, layout, name, str) static Data *layout##_##name(BuiltinArguments&);
eBuiltinMethods(DECLARE_BUILTIN)
#undef DECLARE_BUILTIN

#define DECLARE_BUILTIN(n, name, str) static Data *name(BuiltinArguments&);
ePlainBuiltinFunctions(DECLARE_BUILTIN)
#undef DECLARE_BUILTIN

};

}

#endif // _RUNTIME_BUILTINS
