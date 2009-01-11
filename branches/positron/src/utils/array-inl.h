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

template <typename T, word L>
T &embed_array<T, L>::operator[](word offset) {
  assert offset < L;
  return base_[offset];
}

template <typename T, word L>
const T &embed_array<T, L>::operator[](word offset) const {
  assert offset < L;
  return base_[offset];
}

} // namespace positron

#endif // _UTILS_ARRAY_INL
