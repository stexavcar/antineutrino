#ifndef _UTILS_ARRAY_INL
#define _UTILS_ARRAY_INL

#include "utils/array.h"
#include "utils/check-inl.h"

namespace positron {

template <typename T>
array<T> array<T>::from(word offset) {
  assert offset >= 0;
  assert offset < length_;
  return TO_ARRAY(T, start() + offset, length_ - offset);
}

template <typename T>
T &array<T>::operator[](word offset) {
  assert offset >= 0;
  assert offset < length_;
  return data_[offset];
}

} // namespace positron

#endif // _UTILS_ARRAY_INL
