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
  VISIT(0,  Push,   1)   VISIT(1,  Return, 0)   VISIT(2,  Global,   1) \
  VISIT(3,  Call,   1)   VISIT(4,  Slap,   1)   VISIT(5,  Argument, 1) \
  VISIT(6,  Void,   0)   VISIT(7,  Null,   0)   VISIT(8,  True,     0) \
  VISIT(9,  False,  0)   VISIT(10, Pop,    1)   VISIT(11, IfTrue,  1)  \
  VISIT(12, Goto,   1)   VISIT(13, Invoke, 3)   VISIT(14, Builtin,  2) \
  VISIT(15, Tuple,  1)   VISIT(16, Concat, 1)   VISIT(17, LdLocal, 1)  \
  VISIT(18, ChkHgt, 1)   VISIT(19, Outer,  1)   VISIT(20, Closure,  2) \
  VISIT(21, Quote,  1)   VISIT(22, Unquote, 1)  VISIT(23, Raise,    2) \
  VISIT(24, Mark,   1)   VISIT(25, Unmark, 0)   VISIT(26, New,      1) \
  VISIT(27, LdField, 2)  VISIT(28, IfFalse, 1)  VISIT(29, Task,     0) \
  VISIT(30, Yield,  0)   VISIT(31, StLocal, 1)  VISIT(32, Attach,   0) \
  VISIT(33, Keyword, 1)  VISIT(34, StField, 2)  VISIT(35, InvSup,   4) \
  VISIT(36, Forward, 1)  VISIT(37, BindFor, 0)  VISIT(38, Swap,     0)

enum Opcode {
  __first_opcode = -1
#define DECLARE_OPCODE(n, Name, argc) , oc##Name = n
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
  Data *lookup_super_method(Layout *layout, Selector *selector,
      Signature *current);
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _RUNTIME_INTERPRETER
