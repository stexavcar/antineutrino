#include "plankton/builder-inl.h"
#include "plankton/codec-inl.h"
#include "utils/check-inl.h"

namespace positron {

// --- V a l u e   I m p l e m e n t a t i o n ---


class ValueImpl {
public:
  static plankton::Value::Type value_type(plankton::Value *that);
  static int32_t integer_value(plankton::Integer *that);
  static size_t string_length(plankton::String *that);
private:
  static int8_t *open(plankton::Value *obj);
};

static inline void *tag_object(word offset) {
  return reinterpret_cast<void*>((offset << 1) | 1);
}


static inline word untag_object(void *value) {
  return reinterpret_cast<word>(value) >> 1;
}


static inline void *tag_smi(word value) {
  return reinterpret_cast<void*>(value << 1);
}

plankton::Value::Type ValueImpl::value_type(plankton::Value *that) {
  word value = reinterpret_cast<word>(that->data());
  if ((value & 1) == 0) {
    return plankton::Value::vtInteger;
  } else {
    return plankton::Value::vtInteger;
  }
}

int32_t ValueImpl::integer_value(plankton::Integer *that) {
  return reinterpret_cast<word>(that->data()) >> 1;
}

size_t ValueImpl::string_length(plankton::String *that) {
  int8_t *ptr = ValueImpl::open(that);
  return 6;
}

int8_t *ValueImpl::open(plankton::Value *obj) {
  word offset = untag_object(obj->data());
  return static_cast<DTableImpl&>(obj->dtable()).resolve(offset);
}

DTableImpl DTableImpl::kStaticInstance(NULL);

DTableImpl::DTableImpl(Builder *builder) : builder_(builder) {
  value_type = &ValueImpl::value_type;
  integer_value = &ValueImpl::integer_value;
  string_length = &ValueImpl::string_length;
}

// --- F a c t o r y   m e t h o d s ---


plankton::Integer Builder::new_integer(int32_t value) {
  return plankton::Integer(tag_smi(value), DTableImpl::static_instance());
}

plankton::String Builder::new_string(string value) {
  BuilderStream stream(*this);
  word offset = stream.offset();
  stream.write(plankton::Value::vtString);
  Encoder<BuilderStream>::encode_uint32(value.length(), stream);
  for (size_t i = 0; i < value.length(); i++)
    Encoder<BuilderStream>::encode_uint32(value[i], stream);
  return plankton::String(tag_object(offset), dtable());
}

// --- B u i l d e r ---

Builder::Builder()
  : dtable_(this)
  , is_open_(true) {
}

BuilderStream::BuilderStream(Builder &builder)
    : builder_(builder) {
  assert (builder.is_open()) == true;
  builder.set_is_open(false);
}

BuilderStream::~BuilderStream() {
  builder().set_is_open(true);
}

} // namespace positron
