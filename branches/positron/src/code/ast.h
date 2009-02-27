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
  template <typename T> CodePointer<T> add_instr(const T &obj);
  probably add(s_exp *expr);
  probably register_literal(ref<Value> value, word *index);
  buffer<code_t> &buf() { return buf_; }
  vector<code_t> data() { return buf().as_vector(); }
  word cursor() { return buf().length(); }
  likely<Array> literals();
private:
  Runtime &runtime() { return runtime_; }
  ArrayBuffer &literal_buffer() { return literal_buffer_; }
  Runtime &runtime_;
  buffer<code_t> buf_;
  ArrayBuffer literal_buffer_;
};

} // namespace neutrino

#endif // _CODE_AST
