#ifndef _UTILS_ARRAY_INL
#define _UTILS_ARRAY_INL

#include "utils/array.h"
#include "utils/check-inl.h"

namespace neutrino {

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

template <typename T>
const T &array<T>::operator[](word offset) const {
  assert offset >= 0;
  assert offset < length_;
  return data_[offset];
}

template <typename T>
void array<T>::copy_to(array<T> that, word length) {
  ::memcpy(that.start(), start(), length * sizeof(T));
}

} // namespace neutrino

inline void *operator new(size_t size, neutrino::array<uint8_t> data) {
#ifdef ARRAY_BOUNDS_CHECKS
  assert size <= static_cast<size_t>(data.length());
#endif
  return data.start();
}

#endif // _UTILS_ARRAY_INL
