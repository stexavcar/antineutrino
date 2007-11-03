#ifndef _RUNTIME_INTERPRETER
#define _RUNTIME_INTERPRETER

#include "utils/consts.h"
#include "utils/types.h"

namespace neutrino {

enum Opcode {
  _FIRST_OPCODE = -1
#define DECLARE_OPCODE(NAME, value, argc) , NAME = value
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
  static const uint32_t kLimit = 100;
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
