#ifndef _HEAP_VALUES_INL
#define _HEAP_VALUES_INL

#include "heap/pointer-inl.h"
#include "heap/ref-inl.h"
#include "heap/values.h"
#include "io/ast.h"
#include "utils/checks.h"
#include "utils/types-inl.h"

namespace neutrino {


// ---------------------------
// --- V a l u e   I n f o ---
// ---------------------------

/**
 * The ValueInfo class has a specialization for each type of object
 * which gives static access to various information about the class.
 */
template <class C> class ValueInfo { };

#define SPECIALIZE_VALUE_INFO(n, NAME, Name)                         \
template <> class ValueInfo<Name> {                                  \
public:                                                              \
  static const int kTag = NAME##_TYPE;                               \
};
FOR_EACH_DECLARED_TYPE(SPECIALIZE_VALUE_INFO)
#undef SPECIALIZE_VALUE_INFO


// ---------------------------
// --- P r e d i c a t e s ---
// ---------------------------

template <class To, class From>
inline To *cast(From *from) {
  ASSERT_IS_C(CAST_ERROR, To, from);
  return static_cast<To*>(from);
}

template <class To, class From>
inline ref<To> cast(ref<From> from) {
  ASSERT_IS_C(CAST_ERROR, To, *from);
  return ref<To>(reinterpret_cast<To**>(from.cell()));
}

template <>
inline bool is<Data>(Data *val) {
  return true;
}

template <>
inline bool is<Smi>(Data *val) {
  return ValuePointer::has_smi_tag(val);
}

template <>
inline bool is<Object>(Data *val) {
  return ValuePointer::has_object_tag(val);
}

template <>
inline bool is<Value>(Data *val) {
  return is<Smi>(val) || is<Object>(val);
}

#define DEFINE_QUERY(n, NAME, Name)                                  \
  template <>                                                        \
  inline bool is<Name>(Data *val) {                                  \
    return is<Object>(val)                                           \
        && cast<Object>(val)->chlass()->instance_type() == NAME##_TYPE; \
  }
FOR_EACH_OBJECT_TYPE(DEFINE_QUERY)
#undef DEFINE_QUERY

template <>
inline bool is<AbstractBuffer>(Data *val) {
  return is<Object>(val) && (is<Code>(val) || is<Buffer>(val));
}

template <>
inline bool is<Signal>(Data *val) {
  return ValuePointer::has_signal_tag(val);
}

#define DEFINE_SIGNAL_QUERY(n, NAME, Name)                           \
  template <>                                                        \
  inline bool is<Name>(Data *val) {                                  \
    return is<Signal>(val)                                           \
        && cast<Signal>(val)->type() == Signal::NAME;          \
  }
FOR_EACH_SIGNAL_TYPE(DEFINE_SIGNAL_QUERY)
#undef DEFINE_SIGNAL_QUERY


// -----------------
// --- V a l u e ---
// -----------------

InstanceType Value::type() {
  if (is<Smi>(this)) {
    return SMI_TYPE;
  } else {
    uint32_t result = cast<Object>(this)->chlass()->instance_type();
    return static_cast<InstanceType>(result);
  }
}

string ref_traits<Value>::to_string() {
  return open(this)->to_string();
}

void ref_traits<Value>::print(FILE *out) {
  open(this)->print(out);
}

InstanceType ref_traits<Value>::type() {
  return open(this)->type();
}

// -------------------------------------
// --- A c c e s s o r   M a c r o s ---
// -------------------------------------

#define DEFINE_ACCESSOR(T, Class, name, kOffset)                     \
T &Class::name() {                                                   \
  return ValuePointer::access_field<T>(this, Class::kOffset);        \
}

#define DEFINE_SETTER(T, Class, name, kOffset)                       \
void Class::set_##name(T value) {                                    \
  T &field = ValuePointer::access_field<T>(this, Class::kOffset);    \
  field = value;                                                     \
}

#define DEFINE_ACCESSORS(T, Class, name, kOffset)                    \
  DEFINE_ACCESSOR(T, Class, name, kOffset)                           \
  DEFINE_SETTER(T, Class, name, kOffset)


// ---------------------------------
// --- S m a l l   I n t e g e r ---
// ---------------------------------

Smi* Smi::from_int(int32_t value) {
  return ValuePointer::tag_as_smi(value);
}

int32_t Smi::value() {
  return ValuePointer::value_of(this);
}


// -------------------
// --- O b j e c t ---
// -------------------

DEFINE_ACCESSORS(Class*, Object, chlass, kChlassOffset)


// -------------------
// --- S t r i n g ---
// -------------------

DEFINE_ACCESSOR(uint32_t, String, length, kLengthOffset)

char &String::at(uint32_t index) {
  return ValuePointer::access_field<char>(this, String::kHeaderSize + sizeof(char) * index);
}

uint32_t String::size_for(uint32_t chars) {
  uint32_t raw_size = kHeaderSize + sizeof(char) * chars;
  return ValuePointer::align(raw_size);
}


// -----------------
// --- T u p l e ---
// -----------------

DEFINE_ACCESSOR(uint32_t, Tuple, length, kLengthOffset)

Value *&Tuple::at(uint32_t index) {
  ASSERT_C(OUT_OF_BOUNDS, index < length());
  return ValuePointer::access_field<Value*>(this, Tuple::kHeaderSize + kPointerSize * index);
}

uint32_t Tuple::size_for(uint32_t elms) {
  return Tuple::kHeaderSize + elms * kPointerSize;
}

uint32_t ref_traits<Tuple>::length() {
  return open(this)->length();
}

ref<Value> ref_traits<Tuple>::get(uint32_t index) {
  return new_ref(open(this)->at(index));
}

void ref_traits<Tuple>::set(uint32_t index, ref<Value> value) {
  open(this)->at(index) = *value;
}


// ---------------------------
// --- D i c t i o n a r y ---
// ---------------------------

DEFINE_ACCESSORS(Tuple*, Dictionary, table, kTableOffset)

ref<Value> ref_traits<Dictionary>::get(ref<Value> value) {
  Data *result = open(this)->get(*value);
  if (is<Nothing>(result)) return ref<Value>();
  else return new_ref(cast<Value>(result));
}

void ref_traits<Dictionary>::set(ref<Value> key, ref<Value> value) {
  open(this)->set(*key, *value);
}

uint32_t ref_traits<Dictionary>::size() {
  return open(this)->size();
}

Dictionary::Iterator::Iterator(Dictionary *dict)
    : table_(dict->table())
    , index_(0) {
}

bool Dictionary::Iterator::next(Dictionary::Iterator::Entry *entry) {
  if (index() < table()->length()) {
    entry->key = table()->at(index());
    entry->value = table()->at(index() + 1);
    index_ += 2;
    return true;
  } else {
    return false;
  }
}


// -------------------
// --- L a m b d a ---
// -------------------

DEFINE_ACCESSOR(uint32_t, Lambda, argc, kArgcOffset)
DEFINE_ACCESSORS(Code*, Lambda, code, kCodeOffset)
DEFINE_ACCESSORS(Tuple*, Lambda, literals, kLiteralsOffset)

ref<Code> ref_traits<Lambda>::code() {
  return new_ref(open(this)->code());
}

ref<Tuple> ref_traits<Lambda>::literals() {
  return new_ref(open(this)->literals());
}


// -------------------
// --- B u f f e r ---
// -------------------

template <typename T>
uint32_t AbstractBuffer::size() {
  uint32_t field = ValuePointer::access_field<uint32_t>(this, kSizeOffset);
  return field / sizeof(T);
}

template <typename T>
void AbstractBuffer::set_size(uint32_t size) {
  uint32_t value = size / sizeof(T);
  ValuePointer::access_field<uint32_t>(this, kSizeOffset) = value;
}

template <typename T>
T &AbstractBuffer::at(uint32_t index) {
  ASSERT_C(OUT_OF_BOUNDS, index < size<T>());
  return ValuePointer::access_field<T>(this, kHeaderSize + index * sizeof(T));
}

uint32_t AbstractBuffer::size_for(uint32_t byte_count) {
  uint32_t raw_size = kHeaderSize + byte_count;
  return ValuePointer::align(raw_size);
}


// ---------------
// --- C o d e ---
// ---------------

uint16_t &Code::at(uint32_t index) {
  return AbstractBuffer::at<uint16_t>(index);
}

uint32_t Code::length() {
  return AbstractBuffer::size<uint16_t>();
}

uint16_t &ref_traits<Code>::at(uint32_t index) {
  return open(this)->at(index);
}


// -------------------
// --- M e t h o d ---
// -------------------

DEFINE_ACCESSORS(String*, Method, name, kNameOffset)
DEFINE_ACCESSORS(Lambda*, Method, lambda, kLambdaOffset)


// -----------------
// --- C l a s s ---
// -----------------

DEFINE_ACCESSOR(uint32_t, Class, instance_type, kInstanceTypeOffset)
DEFINE_ACCESSOR(Value*, Class, super, kSuperOffset)

DEFINE_ACCESSORS(Tuple*, Class, methods, kMethodsOffset)


// ---------------------
// --- S i g n a l s ---
// ---------------------

uint32_t Signal::type() {
  return ValuePointer::signal_type(this);
}

uint32_t Signal::payload() {
  return ValuePointer::signal_payload(this);
}

AllocationFailed *AllocationFailed::make(int size) {
  Signal *result = ValuePointer::tag_as_signal(ALLOCATION_FAILED, size);
  return cast<AllocationFailed>(result);
}

InternalError *InternalError::make(int code) {
  Signal *result = ValuePointer::tag_as_signal(INTERNAL_ERROR, code);
  return cast<InternalError>(result);
}

Nothing *Nothing::make() {
  Signal *result = ValuePointer::tag_as_signal(NOTHING, 0);
  return cast<Nothing>(result);
}

PendingRegister *PendingRegister::make(uint32_t reg) {
  Signal *result = ValuePointer::tag_as_signal(PENDING_REGISTER, reg);
  return cast<PendingRegister>(result);
}

uint32_t PendingRegister::index() {
  return payload();
}

} // namespace neutrino

#endif // _HEAP_VALUES_INL
