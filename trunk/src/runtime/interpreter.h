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

class Frame {
public:
  Frame(word *fp) : fp_(fp) { }
  inline uint32_t &prev_pc();
  inline word *&prev_fp();
  inline Lambda *&lambda();

  static const uint32_t kPrevPcOffset = 0;
  static const uint32_t kPrevFpOffset = kPrevPcOffset + 1;
  static const uint32_t kLambdaOffset = kPrevFpOffset + 1;
  static const uint32_t kSize         = kLambdaOffset + 1;

private:
  word *fp_;
};

/**
 * The stack used by the interpreter.
 */

class Stack {
public:
  Stack();
  inline Frame top() { return Frame(fp_); }
  inline Frame push_activation();
  inline Value *pop_value();
  inline Value *peek_value();
  inline void push_value(Value *value);
  inline void push_word(word value);
  uint32_t sp() { return sp_; }
  word *data() { return data_; }
private:
  static const uint32_t kLimit = 100;
  uint32_t sp_;
  word *fp_;
  word data_[kLimit];
};

/**
 * The bytecode interpreter.
 */
class Interpreter {
public:
  Interpreter(Runtime &runtime) : runtime_(runtime) { }
  ref<Value> call(ref<Lambda> lambda);
private:
  ref<Value> interpret(Stack &stack);
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _RUNTIME_INTERPRETER
