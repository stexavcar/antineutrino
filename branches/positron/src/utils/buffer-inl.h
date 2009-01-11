#ifndef _UTILS_BUFFER_INL
#define _UTILS_BUFFER_INL

#include "utils/buffer.h"

namespace positron {

template <typename T>
buffer<T>::buffer() {
  data_ = new T[kInitialCapacity];
  length_ = 0;
  capacity_ = kInitialCapacity;
}

template <typename T>
buffer<T>::~buffer() {
  delete[] data_;
}

template <typename T>
T &buffer<T>::operator[](int index) {
  // ASSERT(index < length());
  return start()[index];
}

template <typename T>
void buffer<T>::append(const T &obj) {
  ensure_capacity(length_ + 1);
  start()[length_++] = obj;
}

template <typename T>
void buffer<T>::push(const T &obj) {
  append(obj);
}

template <typename T>
T buffer<T>::remove_last() {
  // ASSERT(length() > 0);
  T result = start()[length_ - 1];
  length_--;
  return result;
}

template <typename T>
const T &buffer<T>::peek() {
  // ASSERT(length() > 0);
  return start()[length_ - 1];
}

template <typename T>
T buffer<T>::pop() {
  return remove_last();
}

template <typename T>
void buffer<T>::ensure_capacity(int length) {
  if (length > capacity_)
    extend_capacity(length);
}

template <typename T>
void buffer<T>::extend_capacity(int required) {
  int new_capacity = grow_value(required);
  T *new_data = new T[new_capacity];
  for (int i = 0; i < length_; i++)
    new_data[i] = start()[i];
  delete[] data_;
  data_ = new_data;
  capacity_ = new_capacity;
}

} // namespace positron

#endif // _UTILS_BUFFER_INL
