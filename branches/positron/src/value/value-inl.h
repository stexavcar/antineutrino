#ifndef _VALUE_VALUE_INL
#define _VALUE_VALUE_INL

#include "value/pointer-inl.h"
#include "utils/check.h"

namespace positron {

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
static inline bool is<Failure>(Data *data) {
  return is<Signal>(data) && cast<Signal>(data)->type() == Signal::sFailure;
}

Signal::Type Signal::type() {
  return Pointer::signal_type(this);
}

Success *Success::make() {
  return cast<Success>(Pointer::tag_signal(sSuccess));
}

Failure *Failure::make() {
  return cast<Failure>(Pointer::tag_signal(sFailure));
}

} // namespace positron

#endif // _VALUE_VALUE_INL
