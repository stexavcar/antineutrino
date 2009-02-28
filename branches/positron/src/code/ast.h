#ifndef _CODE_AST
#define _CODE_AST

#include "code/instr.h"
#include "utils/buffer.h"
#include "value/condition.h"

namespace neutrino {

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
  template <typename T> CodePointer<T> add(const T &obj);
  probably register_literal(ref<Value> value, word *index);
  word cursor() { return code().length(); }
  likely<SyntaxTree> result();
  Runtime &runtime() { return runtime_; }
  buffer<code_t> &code() { return code_; }
private:
  ArrayBuffer &literals() { return literals_; }
  Runtime &runtime_;
  buffer<code_t> code_;
  ArrayBuffer literals_;
};

class CodeGenerator {
public:
  CodeGenerator(Runtime &runtime) : code_(runtime) { }
  probably emit_expression(s_exp *expr);
  Runtime &runtime() { return code().runtime(); }
  CodeStream &code() { return code_; }
private:
  CodeStream code_;
};

} // namespace neutrino

#endif // _CODE_AST
