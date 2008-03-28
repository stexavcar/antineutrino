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
#define FOR_EACH_OPCODE(VISIT)                                         \
  VISIT(0,  PUSH,   1)   VISIT(1,  RETURN, 0)   VISIT(2,  GLOBAL,   1) \
  VISIT(3,  CALL,   1)   VISIT(4,  SLAP,   1)   VISIT(5,  ARGUMENT, 1) \
  VISIT(6,  VOID,   0)   VISIT(7,  NULL,   0)   VISIT(8,  TRUE,     0) \
  VISIT(9,  FALSE,  0)   VISIT(10, POP,    1)   VISIT(11, IF_TRUE,  1) \
  VISIT(12, GOTO,   1)   VISIT(13, INVOKE, 3)   VISIT(14, BUILTIN,  2) \
  VISIT(15, TUPLE,  1)   VISIT(16, CONCAT, 1)   VISIT(17, LD_LOCAL, 1) \
  VISIT(18, CHKHGT, 1)   VISIT(19, OUTER,  1)   VISIT(20, CLOSURE,  2) \
  VISIT(21, QUOTE,  1)   VISIT(22, UNQUOTE, 1)  VISIT(23, RAISE,    2) \
  VISIT(24, MARK,   1)   VISIT(25, UNMARK, 0)   VISIT(26, NEW,      1) \
  VISIT(27, FIELD,  2)   /* extern was here */  VISIT(29, TASK,     0) \
  VISIT(30, YIELD,  0)   VISIT(31, ST_LOCAL, 1) VISIT(32, ATTACH,   0) \
  VISIT(33, KEYWORD, 1)

enum Opcode {
  __first_opcode = -1
#define DECLARE_OPCODE(n, NAME, argc) , OC_##NAME = n
FOR_EACH_OPCODE(DECLARE_OPCODE)
#undef DECLARE_OPCODE
};

/**
 *
 *             |     . . .     |
 *             +---------------+
 *             |     data      |
 *             +---------------+
 *       mp -> |    prev mp    |
 *             +---------------+
 *             |     . . .     |
 *
 */
class Marker {
public:
  Marker(word *mp) : mp_(mp) { }
  word *mp() { return mp_; }
  inline void set_prev_mp(word *value);
  inline word *prev_mp();
  inline Value *data();
  inline void set_data(Value *value);
  inline bool is_bottom();
  inline void unwind();
  
  static const uword kPrevMpOffset = 0;
  static const uword kDataOffset   = kPrevMpOffset + 1;
  static const uword kSize         = kDataOffset + 1;
private:
  word *mp_;
};

/**
 *             |     . . .     |
 *       pc -> +---------------+
 *             |    local n    |
 *             +---------------+
 *             |     . . .     |
 *             +---------------+
 *             |    local 0    |
 *             +---------------+
 *             |    prev fp    |
 *             +---------------+
 *             |    prev pc    |
 *             +---------------+
 *             |    lambda     |
 *       fp -> +---------------+
 *             |     arg n     |
 *             +---------------+
 *             |     . . .     |
 *             +---------------+
 *             |     arg 0     |
 *             +---------------+
 *             |    method     |
 *             +---------------+
 *             |     self      |
 *             +---------------+
 *             |     . . .     |
 */
class Frame {
public:
  Frame(word *fp, word *sp) : fp_(fp), sp_(sp) { }
  Frame(word *fp) : fp_(fp), sp_(fp + kSize) { }
  uword locals() { return sp() - (fp() + kSize); }
  inline uword &prev_pc();
  inline word *&prev_fp();
  inline Lambda *&lambda();
  inline Value *&local(uword index);
  inline Value *&argument(uword index);
  inline Value *&self(uword argc);
  inline Value *pop(uword height = 1);
  inline Value *&operator[](uword offset);
  inline void push_activation();
  inline Marker push_marker();
  inline Marker pop_marker();
  
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
  static inline uword accessible_below_fp(uword argc) { return argc + 2; }

  static const uword kPrevPcOffset = 0;
  static const uword kPrevFpOffset = kPrevPcOffset + 1;
  static const uword kLambdaOffset = kPrevFpOffset + 1;
  static const uword kSize         = kLambdaOffset + 1;

private:
  word *fp_;
  word *sp_;
};

/**
 * The bytecode interpreter.
 */
class Interpreter {
public:
  Interpreter(Runtime &runtime) : runtime_(runtime) { }
  Value *call(Lambda *lambda, Task *task);
private:
  Frame prepare_call(ref<Task> task, ref<Lambda> lambda, uword argc);
  Data *interpret(Stack *stack, Frame &frame, uword *pc_ptr);
  Layout *get_layout(Immediate *val);
  Data *lookup_method(Layout *layout, Selector *selector);
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _RUNTIME_INTERPRETER
