#ifndef _RUNTIME_INTERPRETER
#define _RUNTIME_INTERPRETER

#include "utils/types.h"

namespace neutrino {

#define FOR_EACH_OPCODE(VISIT)                                       \
  VISIT(PUSH, 0)        VISIT(RETURN, 1)      VISIT(GLOBAL, 2)       \
  VISIT(CALL, 3)
 
enum Opcode {
  _FIRST_OPCODE
#define DECLARE_OPCODE(NAME, value) , NAME = value
FOR_EACH_OPCODE(DECLARE_OPCODE)
#undef DECLARE_OPCODE
};

/**
 * The stack used by the interpreter.
 */

class Stack {
public:
  Stack() : sp_(0) { }
  inline Value *pop_value();
  inline Value *peek_value();
  inline void push_value(Value *value);
  inline void push_word(word value);
  uint32_t sp() { return sp_; }
  word *data() { return data_; }
private:
  static const uint32_t kLimit = 100;
  uint32_t sp_;
  word data_[kLimit];
};

/**
 * The bytecode interpreter.
 */
class Interpreter {
public:
  Interpreter(Runtime &runtime) : runtime_(runtime) { }
  ref<Value> interpret(ref<Lambda> lambda, Stack &stack);
private:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _RUNTIME_INTERPRETER
