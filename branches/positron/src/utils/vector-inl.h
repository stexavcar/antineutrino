#ifndef _UTILS_VECTOR_INL
#define _UTILS_VECTOR_INL

#include "utils/vector.h"
#include "utils/check-inl.h"

namespace positron {

template <typename T>
T &vector<T>::operator[](word offset) {
  assert offset < length_;
  return data_[offset];
}

template <typename T>
const T &vector<T>::operator[](word offset) const {
  assert offset < length_;
  return data_[offset];
}

template <typename T>
void pair<T>::set(const T &a, const T &b) {
  this->operator[](0) = a;
  this->operator[](1) = b;
}

} // namespace positron

#endif // _UTILS_VECTOR_INL
