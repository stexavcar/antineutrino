#ifndef _VALUE_VALUE_INL
#define _VALUE_VALUE_INL

#include "value/pointer-inl.h"
#include "utils/check-inl.h"

namespace neutrino {

template <typename T>
static inline T *cast(Data *data) {
  assert is<T>(data);
  return static_cast<T*>(data);
}

template <>
static inline bool is<Object>(Data *data) {
  return Pointer::is_object(data);
}

template <>
static inline bool is<Signal>(Data *data) {
  return Pointer::is_signal(data);
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

Value::Type Object::type() {
  return descriptor()->instance_type();
}

Signal::Type Signal::type() {
  return Pointer::signal_type(this);
}

word Signal::payload() {
  return Pointer::signal_payload(this);
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
