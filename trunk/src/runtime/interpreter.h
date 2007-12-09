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
  VISIT(0,  PUSH,   1) VISIT(1,  RETURN, 0)   VISIT(2,  GLOBAL,   1) \
  VISIT(3,  CALL,   1) VISIT(4,  SLAP,   1)   VISIT(5,  ARGUMENT, 1) \
  VISIT(6,  VOID,   0) VISIT(7,  NULL,   0)   VISIT(8,  TRUE,     0) \
  VISIT(9,  FALSE,  0) VISIT(10, POP,    1)   VISIT(11, IF_TRUE,  1) \
  VISIT(12, GOTO,   1) VISIT(13, INVOKE, 2)   VISIT(14, BUILTIN,  2) \
  VISIT(15, TUPLE,  1) VISIT(16, CONCAT, 1)   VISIT(17, LOCAL,    1) \
  VISIT(18, CHKHGT, 1) VISIT(19, OUTER,  1)   VISIT(20, CLOSURE,  2) \
  VISIT(21, QUOTE,  1) VISIT(22, UNQUOTE, 1)

enum Opcode {
  __first_opcode = -1
#define DECLARE_OPCODE(n, NAME, argc) , OC_##NAME = n
FOR_EACH_OPCODE(DECLARE_OPCODE)
#undef DECLARE_OPCODE
};

class Frame {
public:
  Frame(word *fp, word *sp) : fp_(fp), sp_(sp) { }
  Frame(word *fp) : fp_(fp), sp_(fp + kSize) { }
  inline uint32_t &prev_pc();
  inline word *&prev_fp();
  inline Lambda *&lambda();
  inline Value *&local(uint32_t index);
  inline Value *&argument(uint32_t index);
  inline Value *&self(uint32_t argc);
  inline Value *pop(uint32_t height = 1);
  inline Value *&operator[](uint32_t offset);
  inline void push_activation();
  
  /**
   * Unwinds a stack frame in a cooked stack.
   */
  inline void unwind();
  
  /**
   * Unwinds a stack frame in an uncooked stack, where the given value
   * is the address of the bottom of the stack.
   */
  inline void unwind(word *bottom);
  
  inline void push(Value *value);
  inline bool is_bottom();
  word *fp() { return fp_; }
  word *sp() { return sp_; }
  void reset(Stack *old_stack, Stack *new_stack);
  
  /**
   * Returns the number of stack entries below the current fp that may
   * be accessed through this stack frame.
   */
  static inline uint32_t accessible_below_fp(uint32_t argc) { return argc + 2; }

  static const uint32_t kPrevPcOffset = 0;
  static const uint32_t kPrevFpOffset = kPrevPcOffset + 1;
  static const uint32_t kLambdaOffset = kPrevFpOffset + 1;
  static const uint32_t kSize         = kLambdaOffset + 1;

private:
  word *fp_;
  word *sp_;
};

class UncookedStackIterator {
public:
  inline UncookedStackIterator(Stack *stack);
  Frame &frame() { return frame_; }
  inline void advance();
  inline bool at_end();
private:
  word *bottom_;
  Frame frame_;
};

/**
 * The bytecode interpreter.
 */
class Interpreter {
public:
  Interpreter(Runtime &runtime) : runtime_(runtime) { }
  Value *call(Lambda *lambda, Task *task);
private:
  Data *interpret(Frame &frame, uint32_t *pc_ptr);
  inline Class *get_class(Value *val);
  inline Data *lookup_method(Class *chlass, Value *name);
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _RUNTIME_INTERPRETER
