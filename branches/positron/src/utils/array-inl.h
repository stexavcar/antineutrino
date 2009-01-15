#ifndef _UTILS_ARRAY_INL
#define _UTILS_ARRAY_INL

#include "utils/array.h"
#include "utils/check-inl.h"

namespace positron {

template <typename T>
T &array<T>::operator[](size_t offset) {
  assert offset < length_;
  return data_[offset];
}

template <typename T>
const T &array<T>::operator[](size_t offset) const {
  assert offset < length_;
  return data_[offset];
}

} // namespace positron

#endif // _UTILS_ARRAY_INL
