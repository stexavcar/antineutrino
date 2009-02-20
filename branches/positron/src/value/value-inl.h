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
static inline bool is<Descriptor>(Data *data) {
  return is<Object>(data) && cast<Object>(data)->type() == Value::tDescriptor;
}

template <>
static inline bool is<String>(Data *data) {
  return is<Object>(data) && cast<Object>(data)->type() == Value::tString;
}

Descriptor *Object::descriptor() {
  Descriptor *result = static_cast<Descriptor*>(cast<Object>(header_));
  printf("Getting descriptor of %x: %x, header: %x\n", this, result, header_);
  return result;
}

void Object::set_descriptor(Descriptor *v) {
  header_ = v;
  printf("Setting descriptor of %x: %x, header: %x\n", this, v, header_);
}

void Object::set_forwarding_header(ForwardPointer *v) {
  header_ = v;
}

template <typename O, typename S>
allocation<Descriptor> DescriptorImpl<O, S>::clone(Space &space) {
  pTryAllocInSpace(space, S, result, (*static_cast<S*>(this)));
  return result;
}

template <typename O, typename S>
word DescriptorImpl<O, S>::size_in_memory(Object *obj) {
  return sizeof(O);
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
  return descriptor()->instance_type();
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
