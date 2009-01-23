#ifndef _VALUE_VALUE_INL
#define _VALUE_VALUE_INL

#include "value/pointer-inl.h"
#include "utils/check.h"

namespace neutrino {

template <typename T>
static inline T *cast(Data *data) {
  assert is<T>(data);
  return reinterpret_cast<T*>(data);
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

} // namespace neutrino

#endif // _VALUE_VALUE_INL
