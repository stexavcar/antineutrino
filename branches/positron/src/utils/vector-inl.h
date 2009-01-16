#ifndef _UTILS_ARRAY_INL
#define _UTILS_ARRAY_INL

#include "utils/vector.h"
#include "utils/check-inl.h"

namespace positron {

template <typename T>
T &vector<T>::operator[](size_t offset) {
  assert offset < length_;
  return data_[offset];
}

template <typename T>
const T &vector<T>::operator[](size_t offset) const {
  assert offset < length_;
  return data_[offset];
}

} // namespace positron

#endif // _UTILS_ARRAY_INL
