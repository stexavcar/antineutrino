#ifndef _UTILS_VECTOR_INL
#define _UTILS_VECTOR_INL

#include "utils/vector.h"
#include "utils/check-inl.h"

namespace neutrino {

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
vector<T> vector<T>::subvector(word start, word end) {
  if (end < 0) end += (length() + 1);
  assert 0 <= start;
  assert start <= length();
  assert 0 <= end;
  assert end <= length();
  assert start <= end;
  return vector<T>(this->start() + start, end - start);
}

template <typename T>
array<T> vector<T>::as_array() {
  return TO_ARRAY(T, data_, length_);
}

template <typename T>
vector<T> vector<T>::allocate(word length) {
  return allocate(new_delete_array_allocator<T>(), length);
}

template <typename T>
template <class A>
vector<T> vector<T>::allocate(A allocator, word length) {
  return vector<T>(allocator.allocate(length), length);
}

template <typename T>
T &vector<T>::first() {
  assert !is_empty();
  return this->operator[](0);
}

template <typename T>
T &vector<T>::last() {
  assert !is_empty();
  return this->operator[](length() - 1);
}

} // namespace neutrino

#endif // _UTILS_VECTOR_INL
