#ifndef _RUNTIME_INTERPRETER
#define _RUNTIME_INTERPRETER

#include "utils/consts.h"
#include "utils/types.h"

namespace neutrino {

// ---------------------
// --- O p c o d e s ---
// ---------------------

/**
 * This index defines all opcodes, their index and the number of
 * arguments they expect.
 */
#define FOR_EACH_OPCODE(VISIT)                                       \
  VISIT(0,  PUSH,  1)   VISIT(1,  RETURN, 0) VISIT(2,  GLOBAL,   1)  \
  VISIT(3,  CALL,  1)   VISIT(4,  SLAP,   1) VISIT(5,  ARGUMENT, 1)  \
  VISIT(6,  VOID,  0)   VISIT(7,  NULL,  0)  VISIT(8,  TRUE,     0)  \
  VISIT(9,  FALSE, 0)   VISIT(10, POP,    1) VISIT(11, IF_TRUE,  1)  \
  VISIT(12, GOTO,  1)   VISIT(13, INVOKE, 2) VISIT(14, BUILTIN,  2)  \
  VISIT(15, TUPLE, 1)

enum Opcode {
  __first__opcode = -1
#define DECLARE_OPCODE(n, NAME, argc) , OC_##NAME = n
FOR_EACH_OPCODE(DECLARE_OPCODE)
#undef DECLARE_OPCODE
};

class Frame {
public:
  Frame(word *fp) : fp_(fp) { }
  inline uint32_t &prev_pc();
  inline word *&prev_fp();
  inline Lambda *&lambda();
  word *fp() { return fp_; }

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
  inline Stack();
  Frame top() { return Frame(fp_); }
  inline Frame push_activation();
  inline Frame pop_activation();
  inline Value *pop(uint32_t height = 1);
  inline Value *&operator[](uint32_t offset);
  inline Value *&argument(uint32_t index);
  inline Value *&self(uint32_t argc);
  inline void push(Value *value);
  word *&sp() { return sp_; }
  word *bottom() { return data_; }
private:
  static const uint32_t kLimit = 2048;
  word *sp_;
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
  inline Class *get_class(Value *val);
  inline Data *lookup_method(Class *chlass, Value *name);
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _RUNTIME_INTERPRETER
