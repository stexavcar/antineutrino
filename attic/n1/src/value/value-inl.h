#ifndef _VALUE_VALUE_INL
#define _VALUE_VALUE_INL

#include "value/condition.h"
#include "value/pointer-inl.h"
#include "utils/check-inl.h"

namespace neutrino {

template <typename T>
static inline T *cast(Data *data) {
  assert is<T>(data);
  return static_cast<T*>(data);
}

/* --- *
 * P r e d i c a t e s
 * --- */

template <>
static inline bool is<Object>(Data *data) {
  return Pointer::is_object(data);
}

template <>
static inline bool is<SmallDouble>(Data *data) {
  return IF_ELSE(cc32, false, Pointer::is_small_double(data));
}

template <>
static inline bool is<Signal>(Data *data) {
  return Pointer::is_signal(data);
}

template <>
static inline bool is<ForwardPointer>(Data *data) {
  return Pointer::is_forward_pointer(data);
}

template <>
static inline bool is<Success>(Data *data) {
  return is<Signal>(data) && cast<Signal>(data)->type() == Signal::sSuccess;
}

template <>
static inline bool is<InternalError>(Data *data) {
  return is<Signal>(data) && cast<Signal>(data)->type() == Signal::sInternalError;
}

template <>
static inline bool is<FatalError>(Data *data) {
  return is<Signal>(data) && cast<Signal>(data)->type() == Signal::sFatalError;
}

#define MAKE_OBJECT_PREDICATE(__Type__)                              \
  template <>                                                        \
  static inline bool is<__Type__>(Data *data) {                      \
    return is<Object>(data)                                          \
        && cast<Object>(data)->type() == Value::t##__Type__;         \
  }
eConcreteValueTypes(MAKE_OBJECT_PREDICATE)
#undef MAKE_OBJECT_PREDICATE

class DataFormatter : public variant_type {
public:
  static DataFormatter &instance() { return kInstance; }
  virtual void print_on(const variant &that, string modifiers,
      string_stream &stream);
private:
  static DataFormatter kInstance;
};

inline void encode_variant(variant &that, Data *value) {
  that.type_ = &DataFormatter::instance();
  that.data_.u_ptr = value;
}

#ifdef M64
double SmallDouble::value() {
  return Pointer::untag_small_double(this);
}
#endif

/* --- T a g g e d   I n t e g e r --- */

template <>
static inline bool is<TaggedInteger>(Data *data) {
  return Pointer::is_tagged_integer(data);
}

bool TaggedInteger::fits(word value) {
  return Pointer::fits_tagged_integer(value);
}

TaggedInteger *TaggedInteger::make(word value) {
  return Pointer::tag_integer(value);
}

word TaggedInteger::value() {
  return Pointer::tagged_integer_value(this);
}

/* --- O b j e c t --- */

Species *Object::species() {
  Species *result = static_cast<Species*>(cast<Object>(header_));
  return result;
}

void Object::set_species(Species *v) {
  header_ = v;
}

void Object::set_forwarding_header(ForwardPointer *v) {
  header_ = v;
}

/* --- A r r a y --- */

word Array::size_in_memory(word elements) {
  return sizeof(Array) + sizeof(Value*) * elements;
}

word Instance::size_in_memory(word fields) {
  return sizeof(Instance) + (sizeof(Value*) * fields);
}

array<Value*> Array::elements() {
  uint8_t *start = reinterpret_cast<uint8_t*>(this) + sizeof(Array);
  return TO_ARRAY(Value*, reinterpret_cast<Value**>(start), length_);
}

vector<Value*> Array::as_vector() {
  return vector<Value*>(elements(), length());
}

void Array::set(word index, Value *value) {
  elements()[index] = value;
}

Value *Array::get(word index) {
  return elements()[index];
}

/* --- S t r i n g --- */

word String::size_in_memory(word chars) {
  return sizeof(String) + sizeof(code_point) * chars ;
}

array<code_point> String::chars() {
  uint8_t *start = reinterpret_cast<uint8_t*>(this) + sizeof(String);
  return TO_ARRAY(code_point, reinterpret_cast<code_point*>(start), length_);
}

vector<code_point> String::as_vector() {
  return vector<code_point>(chars(), length());
}

Value::Type Object::type() {
  return species()->instance_type();
}

/* --- B l o b --- */

word Blob::size_in_memory(word length) {
  return sizeof(Blob) + length;
}

template <typename T>
array<T> Blob::data() {
  uint8_t *start = reinterpret_cast<uint8_t*>(this) + sizeof(Blob);
  return TO_ARRAY(T, reinterpret_cast<T*>(start), length_ / sizeof(T));
}

template <typename T>
vector<T> Blob::as_vector() {
  return vector<T>(data<T>(), length_ / sizeof(T));
}

/* --- S i g n a l --- */

Signal::Type Signal::type() {
  return static_cast<Signal::Type>(Pointer::signal_type(this));
}

word Signal::payload() {
  return Pointer::signal_payload(this);
}

Object *ForwardPointer::target() {
  return Pointer::forward_pointer_target(this);
}

ForwardPointer *ForwardPointer::make(Object *value) {
  return Pointer::tag_forward_pointer(value);
}

Success *Success::make() {
  return cast<Success>(Pointer::tag_signal(sSuccess, 0));
}

#define MAKE_CONSTRUCTOR(__Name__, __name__)                         \
InternalError *InternalError::__name__() {                           \
  return make(ie##__Name__);                                         \
}
eInternalErrorTypes(MAKE_CONSTRUCTOR)
#undef MAKE_CONSTRUCTOR

InternalError *InternalError::make(InternalError::Type type) {
  return cast<InternalError>(Pointer::tag_signal(sInternalError, type));
}

FatalError *FatalError::out_of_memory() {
  return make(feOutOfMemory);
}

FatalError *FatalError::abort() {
  return make(feAbort);
}

FatalError *FatalError::make(FatalError::Error type) {
  return cast<FatalError>(Pointer::tag_signal(sFatalError, type));
}

FatalError::Error FatalError::error() {
  return static_cast<Error>(payload());
}

} // namespace neutrino

#endif // _VALUE_VALUE_INL
