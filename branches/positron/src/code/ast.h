#ifndef _CODE_AST
#define _CODE_AST

#include "runtime/ref.h"
#include "utils/array.h"
#include "utils/buffer.h"
#include "utils/global.h"
#include "value/condition.h"
#include "value/value.h"

namespace neutrino {

#define eInstructions(VISIT)                                         \
  VISIT(literal, Literal)  VISIT(global, Global)                     \
  VISIT(send, Send)        VISIT(if_false, IfFalse)                  \
  VISIT(ghoto, Goto)       VISIT(if_true_false, IfTrueFalse)

template <word S>
class instr {
public:
  enum Type {
    __first
#define MAKE_ENUM(name, Name) , it##Name
    eInstructions(MAKE_ENUM)
#undef MAKE_ENUM
  };
  explicit instr(Type op) { code()[0] = op; }
  explicit instr(Type op, code_t arg) { code()[0] = op; code()[1] = arg; }
  explicit instr(Type op, code_t arg0, code_t arg1) { code()[0] = op; code()[1] = arg0; code()[2] = arg1; }
  explicit instr(Type op, code_t arg0, code_t arg1, code_t arg2) { code()[0] = op; code()[1] = arg0; code()[2] = arg1; code()[3] = arg2; }
  word size() const { return kHeaderSize; }
  Type type() { return static_cast<Type>(code()[0]); }
  static const word kArgCount = S;
  const code_t *code() const { return code_; }
  code_t *code() { return code_; }
  static const word kHeaderSize = kArgCount + 1;
private:
  code_t code_[kHeaderSize];
};

class literal : public instr<1> {
public:
  explicit literal(code_t index) : instr<1>(itLiteral, index) { }
  inline code_t index() { return code()[1]; }
  void disassemble(DisassembleContext &context);
};

class global : public instr<1> {
public:
  explicit global(code_t name) : instr<1>(itGlobal, name) { }
  inline code_t name() { return code()[1]; }
  void disassemble(DisassembleContext &context);
};

class send : public instr<3> {
public:
  send(code_t name, code_t recv, code_t argc) : instr<3>(itSend, name, recv, argc) { }
  word size() const { return kHeaderSize + argc(); }
  inline code_t name() { return code()[1]; }
  inline code_t receiver() { return code()[2]; }
  inline code_t argc() const { return code()[3]; }
  inline array<code_t> args() { return TO_ARRAY(code_t, code() + kHeaderSize, argc()); }
  void disassemble(DisassembleContext &context);
};

class if_false : public instr<1> {
public:
  explicit if_false(code_t target) : instr<1>(itIfFalse, target) { }
  inline code_t target() { return code()[1]; }
  inline void set_target(code_t value) { code()[1] = value; }
  void disassemble(DisassembleContext &context);
};

class if_true_false : public instr<3> {
public:
  if_true_false(code_t cond, code_t then_part, code_t else_part)
    : instr<3>(itIfTrueFalse, cond, then_part, else_part) { }
  inline code_t condition() { return code()[1]; }
  inline code_t then_part() { return code()[2]; }
  inline code_t else_part() { return code()[3]; }
  void disassemble(DisassembleContext &context);
};

class ghoto : public instr<1> {
public:
  explicit ghoto(code_t target) : instr<1>(itGoto, target) { }
  inline code_t target() { return code()[1]; }
  inline void set_target(code_t value) { code()[1] = value; }
  void disassemble(DisassembleContext &context);
};

class ArrayBuffer {
public:
  ArrayBuffer() : cursor_(0) { }
  probably add(Runtime &runtime, ref<Value> value);
  word length() { return cursor_; }
  persistent<Array> store() { return store_; }
private:
  persistent<Array> store_;
  word cursor_;
};

template <typename T>
class CodePointer {
public:
  CodePointer(word offset, CodeStream &stream)
    : offset_(offset)
    , stream_(stream) { }
  T *operator->();
private:
  CodeStream &stream() { return stream_; }
  word offset() { return offset_; }
  word offset_;
  CodeStream &stream_;
};

class CodeStream {
public:
  explicit CodeStream(Runtime &runtime) : runtime_(runtime) { }
  template <typename T> CodePointer<T> add_instr(const T &obj);
  probably add(s_exp *expr);
  probably register_literal(ref<Value> value, word *index);
  buffer<code_t> &buf() { return buf_; }
  vector<code_t> data() { return buf().as_vector(); }
  word cursor() { return buf().length(); }
  likely<Array> literals();
  static bool disassemble(vector<code_t> code, ref<Array> literals,
      string_stream &out);
private:
  Runtime &runtime() { return runtime_; }
  ArrayBuffer &literal_buffer() { return literal_buffer_; }
  Runtime &runtime_;
  buffer<code_t> buf_;
  ArrayBuffer literal_buffer_;
};

} // namespace neutrino

#endif // _CODE_AST
