#include "io/miniheap-inl.h"
#include "plankton/codec-inl.h"
#include "utils/array-inl.h"
#include "utils/check-inl.h"

namespace neutrino {

// --- V a l u e   I m p l e m e n t a t i o n ---


class ValueImpl {
public:
  static p::Value::Type value_type(p::Value that);
  static bool value_eq(p::Value that, p::Value other);
  static void *value_impl_id(p::Value that);
  static int32_t integer_value(p::Integer that);
  static word string_length(p::String that);
  static uint32_t string_get(p::String that, word index);
  static word string_compare(p::String that, p::String other);
  static word array_length(p::Array that);
  static p::Value array_get(p::Array that, word index);
  static bool array_set(p::Array that, word index, p::Value value);
private:
  static MiniHeapObject open(p::Value obj);
};

p::Value::Type ValueImpl::value_type(p::Value that) {
  uint32_t data = that.data();
  if (MiniHeapPointer::has_integer_tag(data)) {
    return p::Value::vtInteger;
  } else if (MiniHeapPointer::has_object_tag(data)) {
    MiniHeapObject obj = open(that);
    return static_cast<p::Value::Type>(obj.at<uint32_t>(0));
  } else {
    assert MiniHeapPointer::has_singleton_tag(data);
    return MiniHeapPointer::untag_singleton(data);
  }
}

bool ValueImpl::value_eq(p::Value that, p::Value other) {
  return that.data() == other.data();
}

void *ValueImpl::value_impl_id(p::Value that) {
  return &MiniHeapDTable::static_instance();
}

int32_t ValueImpl::integer_value(p::Integer that) {
  return MiniHeapPointer::untag_integer(that.data());
}

word ValueImpl::string_length(p::String that) {
  return ValueImpl::open(that).at<uint32_t>(1);
}

uint32_t ValueImpl::string_get(p::String that, word index) {
  assert index >= 0;
  if (index < that.length()) {
    return ValueImpl::open(that).at<uint32_t>(2 + index);
  } else {
    assert index == that.length();
    return '\0';
  }
}

word ValueImpl::string_compare(p::String that, p::String other) {
  if (that.length() != other.length())
    return that.length() - other.length();
  MiniHeapObject str = ValueImpl::open(that);
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

bool ValueImpl::array_set(p::Array that, word index, p::Value value) {
  assert index >= 0;
  assert index < that.length();
  ValueImpl::open(that).at<uint32_t>(2 + index) = value.data();
  return true;
}

word ValueImpl::array_length(p::Array that) {
  return ValueImpl::open(that).at<uint32_t>(1);
}

p::Value ValueImpl::array_get(p::Array that, word index) {
  assert index >= 0;
  assert index < that.length();
  uint32_t data = ValueImpl::open(that).at<uint32_t>(2 + index);
  return p::Value(data, that.dtable());
}

void MiniHeap::take_ownership(MiniHeap *that) {
  assert !owned_.is_set();
  owned_.set(that);
}

MiniHeapObject MiniHeap::resolve(word offset) {
  return MiniHeapObject(memory().as_array().from(offset));
}

MiniHeap::MiniHeap()
  : dtable_(this) {
}

MiniHeapObject ValueImpl::open(p::Value obj) {
  word offset = MiniHeapPointer::untag_object(obj.data());
  return static_cast<MiniHeapDTable*>(obj.dtable())->heap().resolve(offset);
}

MiniHeapDTable MiniHeapDTable::kStaticInstance(NULL);

MiniHeapDTable::MiniHeapDTable(MiniHeap *heap) : heap_(heap) {
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

p::Integer Factory::new_integer(int32_t value) {
  return p::Integer(MiniHeapPointer::tag_integer(value), &MiniHeapDTable::static_instance());
}

p::String Factory::new_string(string value) {
  MiniHeapObject obj = allocate(p::Value::vtString, 2 + value.length());
  obj.at<uint32_t>(1) = value.length();
  for (word i = 0; i < value.length(); i++)
    obj.at<uint32_t>(2 + i) = value[i];
  return to_plankton<p::String>(obj);
}

p::Array Factory::new_array(word length) {
  assert length >= 0;
  MiniHeapObject obj = allocate(p::Value::vtArray, 2 + length);
  obj.at<uint32_t>(1) = length;
  uint32_t null = get_null().data();
  for (word i = 0; i < length; i++)
    obj.at<uint32_t>(2 + i) = null;
  return to_plankton<p::Array>(obj);
}

p::Null Factory::get_null() {
  return p::Null(MiniHeapPointer::tag_singleton(p::Value::vtNull),
      &MiniHeapDTable::static_instance());
}

p::Void Factory::get_void() {
  return p::Void(MiniHeapPointer::tag_singleton(p::Value::vtVoid),
      &MiniHeapDTable::static_instance());
}

// --- B u i l d e r ---


MiniHeap *MiniHeap::get(p::Value::DTable *dtable) {
  return dtable ? &static_cast<MiniHeapDTable*>(dtable)->heap() : NULL;
}

vector<uint8_t> Factory::memory() {
  return data().as_vector();
}

MiniHeapObject Factory::allocate(p::Value::Type type, word size) {
  assert size >= 1;
  MiniHeapObject result = MiniHeapObject(data().allocate(sizeof(uint32_t) * size));
  result.at<uint32_t>(0) = type;
  return result;
}

boole MessageIn::reply(p::Value value) {
  assert stream_ != static_cast<void*>(NULL);
  has_replied_ = true;
  return stream_->send_reply(*this, value);
}

void MessageIn::take_ownership(MiniHeap *that) {
  assert !owned_.is_set();
  owned_.set(that);
}

} // namespace neutrino
