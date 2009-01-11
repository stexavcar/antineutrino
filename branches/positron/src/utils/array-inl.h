#ifndef _UTILS_ARRAY_INL
#define _UTILS_ARRAY_INL

#include "utils/array.h"
#include "utils/check-inl.h"

namespace positron {

template <typename T, int L>
T &embed_array<T, L>::operator[](int offset) {
  assert offset < L;
  return base_[offset];
}

template <typename T, int L>
const T &embed_array<T, L>::operator[](int offset) const {
  assert offset < L;
  return base_[offset];
}

} // namespace positron

#endif // _UTILS_ARRAY_INL
