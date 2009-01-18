#ifndef _PLANKTON_BUILDER
#define _PLANKTON_BUILDER

#include "plankton/plankton.h"
#include "utils/array.h"
#include "utils/buffer.h"
#include "utils/string.h"

namespace positron {

class DTableImpl : public p_value::DTable {
public:
  DTableImpl(Builder *builder);
  static DTableImpl &static_instance() { return kStaticInstance; }
  Builder &builder() { return *builder_; }
private:
  Builder *builder_;
  static DTableImpl kStaticInstance;
};

class object {
public:
  object(array<uint8_t> data) : data_(data) { }
  uint8_t *start() { return data().start(); }
  template <typename T>
  inline T &at(word offset);
  array<uint8_t> data() { return data_; }
private:
  array<uint8_t> data_;
};

class Builder {
public:
  Builder();
  static p_integer new_integer(int32_t value);
  static p_null get_null();
  p_string new_string(string value);
  p_array new_array(word length);

  object resolve(word offset);
  buffer<uint8_t> &data() { return data_; }

private:
  template <typename T> T to_plankton(object &obj);
  object allocate(p_value::Type type, word size);
  DTableImpl &dtable() { return dtable_; }
  DTableImpl dtable_;
  buffer<uint8_t> data_;
};

class Raw {
public:
  static inline uint32_t tag_object(word offset);
  static inline word untag_object(uint32_t data);
  static inline bool has_object_tag(uint32_t data);
  static inline uint32_t tag_integer(word value);
  static inline word untag_integer(uint32_t data);
  static inline bool has_integer_tag(uint32_t data);
  static inline uint32_t tag_singleton(p_value::Type type);
  static inline p_value::Type untag_singleton(uint32_t data);
  static inline bool has_singleton_tag(uint32_t data);
  static const word kTagSize = 2;
  static const word kTagMask = (1 << kTagSize) - 1;
  static const uword kIntegerTag = 0;
  static const uword kObjectTag = 1;
  static const uword kSingletonTag = 2;
};

} // namespace positron

#endif // _PLANKTON_BUILDER
