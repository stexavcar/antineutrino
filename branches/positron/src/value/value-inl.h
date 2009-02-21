#ifndef _VALUE_VALUE_INL
#define _VALUE_VALUE_INL

#include "runtime/heap-inl.h"
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

template <>
static inline bool is<Species>(Data *data) {
  return is<Object>(data) && cast<Object>(data)->type() == Value::tSpecies;
}

template <>
static inline bool is<String>(Data *data) {
  return is<Object>(data) && cast<Object>(data)->type() == Value::tString;
}

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

word Instance::size_in_memory(word fields) {
  return sizeof(Instance) + (sizeof(Value*) * fields);
}

word String::size_in_memory(word chars) {
  return sizeof(String) + sizeof(code_point) * chars ;
}

array<code_point> String::chars() {
  uint8_t *start = reinterpret_cast<uint8_t*>(this) + sizeof(String);
  return TO_ARRAY(code_point, reinterpret_cast<code_point*>(start), length_);
}

Value::Type Object::type() {
  return species()->instance_type();
}

Signal::Type Signal::type() {
  return Pointer::signal_type(this);
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

InternalError *InternalError::make(InternalError::Type type) {
  return cast<InternalError>(Pointer::tag_signal(sInternalError, type));
}

FatalError *FatalError::make(FatalError::Type type) {
  return cast<FatalError>(Pointer::tag_signal(sFatalError, type));
}

} // namespace neutrino

#endif // _VALUE_VALUE_INL
