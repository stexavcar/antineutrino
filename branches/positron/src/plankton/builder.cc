#include "plankton/builder-inl.h"
#include "plankton/codec-inl.h"
#include "utils/array-inl.h"
#include "utils/check-inl.h"

namespace positron {

// --- V a l u e   I m p l e m e n t a t i o n ---


class ValueImpl {
public:
  static p_value::Type value_type(const p_value *that);
  static bool value_eq(const p_value *that, p_value other);
  static int32_t integer_value(const p_integer *that);
  static word string_length(const p_string *that);
  static uint32_t string_get(const p_string *that, word index);
  static word string_compare(const p_string *that, const string &other);
  static word array_length(const p_array *that);
  static p_value array_get(const p_array *that, word index);
  static bool array_set(p_array *that, word index, p_value value);
private:
  static object open(const p_value *obj);
};

p_value::Type ValueImpl::value_type(const p_value *that) {
  uint32_t data = that->data();
  if (Raw::has_integer_tag(data)) {
    return p_value::vtInteger;
  } else if (Raw::has_object_tag(data)) {
    object obj = open(that);
    return static_cast<p_value::Type>(obj.at<uint32_t>(0));
  } else {
    assert Raw::has_singleton_tag(data);
    return Raw::untag_singleton(data);
  }
}

bool ValueImpl::value_eq(const p_value *that, p_value other) {
  return that->data() == other.data();
}

int32_t ValueImpl::integer_value(const p_integer *that) {
  return Raw::untag_integer(that->data());
}

word ValueImpl::string_length(const p_string *that) {
  return ValueImpl::open(that).at<uint32_t>(1);
}

uint32_t ValueImpl::string_get(const p_string *that, word index) {
  assert index >= 0;
  assert index < that->length();
  return ValueImpl::open(that).at<uint32_t>(2 + index);
}

word ValueImpl::string_compare(const p_string *that, const string &other) {
  if (that->length() != other.length())
    return that->length() - other.length();
  object str = ValueImpl::open(that);
  for (word i = 0; i < other.length(); i++) {
    uint32_t ac = str.at<uint32_t>(2 + i);
    uint32_t bc = static_cast<uint32_t>(other[i]);
    if (ac != bc)
      return ac - bc;
  }
  return 0;
}

bool ValueImpl::array_set(p_array *that, word index, p_value value) {
  assert index >= 0;
  assert index < that->length();
  ValueImpl::open(that).at<uint32_t>(2 + index) = value.data();
  return true;
}

word ValueImpl::array_length(const p_array *that) {
  return ValueImpl::open(that).at<uint32_t>(1);
}

p_value ValueImpl::array_get(const p_array *that, word index) {
  assert index >= 0;
  assert index < that->length();
  uint32_t data = ValueImpl::open(that).at<uint32_t>(2 + index);
  return p_value(data, that->dtable());
}

object MessageBuffer::resolve(word offset) {
  return object(data().raw_data().from(offset));
}

object ValueImpl::open(const p_value *obj) {
  word offset = Raw::untag_object(obj->data());
  return static_cast<DTableImpl&>(obj->dtable()).builder().resolve(offset);
}

DTableImpl DTableImpl::kStaticInstance(NULL);

DTableImpl::DTableImpl(MessageBuffer *builder) : builder_(builder) {
  value_type = &ValueImpl::value_type;
  value_eq = &ValueImpl::value_eq;
  integer_value = &ValueImpl::integer_value;
  string_length = &ValueImpl::string_length;
  string_get = &ValueImpl::string_get;
  string_compare = &ValueImpl::string_compare;
  array_length = &ValueImpl::array_length;
  array_get = &ValueImpl::array_get;
  array_set = &ValueImpl::array_set;
}

// --- F a c t o r y   m e t h o d s ---

p_integer MessageBuffer::new_integer(int32_t value) {
  return p_integer(Raw::tag_integer(value), DTableImpl::static_instance());
}

p_string MessageBuffer::new_string(string value) {
  object obj = allocate(p_value::vtString, 2 + value.length());
  obj.at<uint32_t>(1) = value.length();
  for (word i = 0; i < value.length(); i++)
    obj.at<uint32_t>(2 + i) = value[i];
  return to_plankton<p_string>(obj);
}

p_array MessageBuffer::new_array(word length) {
  assert length >= 0;
  object obj = allocate(p_value::vtArray, 2 + length);
  obj.at<uint32_t>(1) = length;
  uint32_t null = get_null().data();
  for (word i = 0; i < length; i++)
    obj.at<uint32_t>(2 + i) = null;
  return to_plankton<p_array>(obj);
}

p_null MessageBuffer::get_null() {
  return p_null(Raw::tag_singleton(p_value::vtNull), DTableImpl::static_instance());
}

// --- B u i l d e r ---

MessageBuffer::MessageBuffer()
  : dtable_(this) {
}

object MessageBuffer::allocate(p_value::Type type, word size) {
  assert size >= 1;
  object result = object(data().allocate(sizeof(uint32_t) * size));
  result.at<uint32_t>(0) = type;
  return result;
}

bool MessageBuffer::send(p_value value, ISocket &socket) {
  uint32_t ptr = value.data();
  vector<uint8_t> ptr_vector(reinterpret_cast<uint8_t*>(&ptr), sizeof(uint32_t));
  socket.write(ptr_vector);
  socket.write(data().as_vector());
  return true;
}

p_value MessageBuffer::receive(ISocket &socket) {
  uint32_t ptr = 0;
  vector<uint8_t> ptr_vector(reinterpret_cast<uint8_t*>(&ptr), sizeof(uint32_t));
  socket.read(ptr_vector);
  return p_value(ptr, dtable());
}

} // namespace positron
