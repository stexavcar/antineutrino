#ifndef _RUNTIME_INTERPRETER
#define _RUNTIME_INTERPRETER

#include "utils/consts.h"
#include "utils/smart-ptrs-inl.h"
#include "utils/types.h"

namespace neutrino {

// ---------------------
// --- O p c o d e s ---
// ---------------------

/**
 * This index defines all opcodes, their index and the number of
 * arguments they expect.
 */
#define eOpcodes(VISIT)                                                \
  VISIT(0,  Push,        1, "p")   VISIT(1,  Return,        0, "")     \
  VISIT(2,  Global,      1, "p")   VISIT(3,  Call,          1, ".")    \
  VISIT(4,  Slap,        1, "i")   VISIT(5,  Argument,      1, "i")    \
  VISIT(6,  Void,        0, "")    VISIT(7,  Null,          0, "")     \
  VISIT(8,  True,        0, "")    VISIT(9,  False,         0, "")     \
  VISIT(10, Pop,         1, "i")   VISIT(11, IfTrue,        1, "@")    \
  VISIT(12, Goto,        1, "@")   VISIT(13, Invoke,        3, "p..")  \
  VISIT(14, Builtin,     2, "i")   VISIT(15, Tuple,         1, "i")    \
  VISIT(16, Concat,      1, "i")   VISIT(17, LoadLocal,     1, "i")    \
  VISIT(18, CheckHeight, 1, "i")   VISIT(19, Outer,         1, "i")    \
  VISIT(20, Closure,     2, "??")                                      \
  VISIT(22, Unquote,     1, "?")   VISIT(23, Raise,         2, "??")   \
  VISIT(24, Mark,        1, "?")   VISIT(25, Unmark,        0, "")     \
  VISIT(26, New,         1, "p")   VISIT(27, LoadField,     2, "i.")   \
  VISIT(28, IfFalse,     1, "@")   VISIT(29, Task,          0, "")     \
  VISIT(30, Yield,       0, "")    VISIT(31, StoreLocal,    1, "i")    \
  VISIT(32, Attach,      0, "")    VISIT(33, Keyword,       1, "?")    \
  VISIT(34, StoreField,  2, "i.")  VISIT(35, InvokeSuper,   4, "????") \
  VISIT(36, Forward,     1, "?")   VISIT(37, BindForwarder, 0, "")     \
  VISIT(38, Swap,        0, "")    VISIT(39, LazyCompile,   0, "")     \
  VISIT(40, StoreCell,   0, "")    VISIT(41, LoadCell,      0, "")     \
  VISIT(42, NewCell,     0, "")    VISIT(43, StackBottom,   0, "")

enum Opcode {
  __first_opcode = -1
#define DECLARE_OPCODE(n, Name, argc, argf) , oc##Name = n
eOpcodes(DECLARE_OPCODE)
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
class StackState {
public:
  StackState(bounded_ptr<word> fp, bounded_ptr<word>sp)
      : fp_(fp), sp_(sp) { }
  StackState(bounded_ptr<word> fp)
      : fp_(fp), sp_(fp + kSize) { }
  uword locals() { return sp() - (fp() + kSize); }
  inline uword &prev_pc();
  inline word *&prev_fp();
  inline Lambda *&lambda();
  inline Value *&local(uword index);
  inline Value *&argument(uword index);
  inline Value *&self(uword argc);
  inline Value *pop(uword height = 1);
  inline Value *&operator[](uword offset);
  inline void push_activation(uword prev_pc, Lambda *lambda);
  inline Marker push_marker();
  inline Marker pop_marker();

  void park(Stack *stack, uword pc);
  inline uword unpark(Stack *stack);

  /**
   * Unwinds a stack frame in a cooked stack.
   */
  inline void unwind(Stack *stack);

  /**
   * Unwinds a stack frame in an uncooked stack, where the given
   * array is the stack buffer.
   */
  inline void unwind(array<word> buffer, uword height);

  inline void push(Value *value);
  inline bool is_bottom();
  bounded_ptr<word> fp() { return fp_; }
  bounded_ptr<word> sp() { return sp_; }

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
  friend class Stack;

  bounded_ptr<word> fp_;
  bounded_ptr<word> sp_;
};

class OpcodeData {
public:
  OpcodeData() : is_resolved_(false) { }
  void load(uint16_t value);
  bool is_resolved() { return is_resolved_; }
  uword length() { return length_; }
  string name() { return name_; }
  string format() { return format_; }
private:
  bool is_resolved_;
  string name_, format_;
  uword length_;
};

/**
 * The bytecode interpreter.
 */
class Interpreter {
public:
  Interpreter(Runtime &runtime) : runtime_(runtime) { }
  Value *call(ref<Lambda> lambda, ref<Task> task);
private:
  Signal *prepare_call(ref<Task> task, ref<Lambda> lambda, uword argc);
  Data *interpret(InterpreterState &state);
  Layout *get_layout(Immediate *val);
  Data *lookup_method(Immediate *recv, Layout *layout, Selector *selector);
  Data *lookup_super_method(Layout *layout, Selector *selector,
      Signature *current);
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

}

#endif // _RUNTIME_INTERPRETER
