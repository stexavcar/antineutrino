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
  static void *value_impl_id(const p_value *that);
  static int32_t integer_value(const p_integer *that);
  static word string_length(const p_string *that);
  static uint32_t string_get(const p_string *that, word index);
  static word string_compare(const p_string *that, const string &other);
  static word array_length(const p_array *that);
  static p_value array_get(const p_array *that, word index);
  static bool array_set(p_array *that, word index, p_value value);
private:
  static FrozenObject open(const p_value *obj);
};

p_value::Type ValueImpl::value_type(const p_value *that) {
  uint32_t data = that->data();
  if (Raw::has_integer_tag(data)) {
    return p_value::vtInteger;
  } else if (Raw::has_object_tag(data)) {
    FrozenObject obj = open(that);
    return static_cast<p_value::Type>(obj.at<uint32_t>(0));
  } else {
    assert Raw::has_singleton_tag(data);
    return Raw::untag_singleton(data);
  }
}

bool ValueImpl::value_eq(const p_value *that, p_value other) {
  return that->data() == other.data();
}

void *ValueImpl::value_impl_id(const p_value *that) {
  return &DTableImpl::static_instance();
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
  FrozenObject str = ValueImpl::open(that);
  for (word i = 0; i < other.length(); i++) {
    uint32_t ac = str.at<uint32_t>(2 + i);
    uint32_t bc = static_cast<uint32_t>(other[i]);
    if (ac != bc) {
      // The values have to be cast to words first, or a 32-bit negative
      // number may be zero-extended to 64 bits, making it positive.
      return static_cast<word>(ac) - static_cast<word>(bc);
    }
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

FrozenObject MessageHeap::resolve(word offset) {
  return FrozenObject(memory().as_array().from(offset));
}

MessageHeap::MessageHeap()
  : dtable_(this) {
}

FrozenObject ValueImpl::open(const p_value *obj) {
  word offset = Raw::untag_object(obj->data());
  return static_cast<DTableImpl*>(obj->dtable())->heap().resolve(offset);
}

DTableImpl DTableImpl::kStaticInstance(NULL);

DTableImpl::DTableImpl(MessageHeap *heap) : heap_(heap) {
  value.type = &ValueImpl::value_type;
  value.eq = &ValueImpl::value_eq;
  value.impl_id = &ValueImpl::value_impl_id;
  integer.value = &ValueImpl::integer_value;
  string.length = &ValueImpl::string_length;
  string.get = &ValueImpl::string_get;
  string.compare = &ValueImpl::string_compare;
  array.length = &ValueImpl::array_length;
  array.get = &ValueImpl::array_get;
  array.set = &ValueImpl::array_set;
}

// --- F a c t o r y   m e t h o d s ---

p_integer MessageOut::new_integer(int32_t value) {
  return p_integer(Raw::tag_integer(value), &DTableImpl::static_instance());
}

p_string MessageOut::new_string(string value) {
  FrozenObject obj = allocate(p_value::vtString, 2 + value.length());
  obj.at<uint32_t>(1) = value.length();
  for (word i = 0; i < value.length(); i++)
    obj.at<uint32_t>(2 + i) = value[i];
  return to_plankton<p_string>(obj);
}

p_array MessageOut::new_array(word length) {
  assert length >= 0;
  FrozenObject obj = allocate(p_value::vtArray, 2 + length);
  obj.at<uint32_t>(1) = length;
  uint32_t null = get_null().data();
  for (word i = 0; i < length; i++)
    obj.at<uint32_t>(2 + i) = null;
  return to_plankton<p_array>(obj);
}

p_null MessageOut::get_null() {
  return p_null(Raw::tag_singleton(p_value::vtNull), &DTableImpl::static_instance());
}

p_void MessageOut::get_void() {
  return p_void(Raw::tag_singleton(p_value::vtVoid), &DTableImpl::static_instance());
}

// --- B u i l d e r ---


MessageHeap *MessageHeap::get(p_value::DTable *dtable) {
  return dtable ? &static_cast<DTableImpl*>(dtable)->heap() : NULL;
}

vector<uint8_t> MessageOut::memory() {
  return data().as_vector();
}

FrozenObject MessageOut::allocate(p_value::Type type, word size) {
  assert size >= 1;
  FrozenObject result = FrozenObject(data().allocate(sizeof(uint32_t) * size));
  result.at<uint32_t>(0) = type;
  return result;
}

bool MessageIn::reply(p_value value) {
  assert stream_ != static_cast<void*>(NULL);
  has_replied_ = true;
  return stream_->send_reply(*this, value);
}

} // namespace positron
