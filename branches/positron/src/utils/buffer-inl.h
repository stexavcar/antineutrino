#ifndef _UTILS_BUFFER_INL
#define _UTILS_BUFFER_INL

#include "utils/buffer.h"
#include "utils/array.h"

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
T &buffer<T>::operator[](word index) {
  assert index < capacity_;
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
  assert length() > 0;
  T result = start()[length_ - 1];
  length_--;
  return result;
}

template <typename T>
const T &buffer<T>::peek() {
  assert length() > 0;
  return start()[length_ - 1];
}

template <typename T>
T buffer<T>::pop() {
  return remove_last();
}

template <typename T>
array<T> buffer<T>::allocate(word size) {
  ensure_capacity(length_ + size);
  array<T> result = TO_ARRAY(T, start() + length_, size);
  length_ += size;
  return result;
}

template <typename T>
void buffer<T>::ensure_capacity(word length) {
  if (length > capacity_)
    extend_capacity(length);
}

template <typename T>
void buffer<T>::extend_capacity(word required) {
  word new_capacity = grow_value(required);
  T *new_data = new T[new_capacity];
  for (word i = 0; i < length_; i++)
    new_data[i] = start()[i];
  delete[] data_;
  data_ = new_data;
  capacity_ = new_capacity;
}

template <typename T>
array<T> buffer<T>::raw_data() {
  return TO_ARRAY(T, start(), length());
}

template <typename T>
vector<T> buffer<T>::as_vector() {
  return vector<T>(start(), length());
}

} // namespace positron

#endif // _UTILS_BUFFER_INL
