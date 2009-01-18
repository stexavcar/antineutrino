#ifndef _PLANKTON_BUILDER
#define _PLANKTON_BUILDER

#include "plankton/plankton.h"
#include "utils/array.h"
#include "utils/buffer.h"
#include "utils/string.h"

namespace positron {

class DTableImpl : public plankton::Value::DTable {
public:
  DTableImpl(Builder *builder);
  static DTableImpl &static_instance() { return kStaticInstance; }
  Builder &builder() { return *builder_; }
private:
  Builder *builder_;
  static DTableImpl kStaticInstance;
};

class Builder {
public:
  Builder();
  plankton::Integer new_integer(int32_t value);
  plankton::String new_string(string value);

  uint8_t *resolve(word offset);
  bool is_open() { return is_open_; }
  void set_is_open(bool v) { is_open_ = v; }
  buffer<uint8_t> &data() { return data_; }
private:
  DTableImpl &dtable() { return dtable_; }
  DTableImpl dtable_;
  buffer<uint8_t> data_;
  bool is_open_;
};

class BuilderStream {
public:
  BuilderStream(Builder &builder);
  ~BuilderStream();
  inline void write(uint8_t value);
  word offset() { return builder().data().length(); }
private:
  Builder &builder() { return builder_; }
  Builder &builder_;
};

} // namespace positron

#endif // _PLANKTON_BUILDER
