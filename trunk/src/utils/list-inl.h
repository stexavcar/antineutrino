#ifndef _UTILS_LIST_INL
#define _UTILS_LIST_INL

#include "utils/checks.h"
#include "utils/list.h"

namespace neutrino {

template <typename T>
list<T>::list()
    : elms_(NULL)
    , length_(0) { }

template <typename T>
list<T>::list(T elms[], uint32_t length)
    : elms_(elms)
    , length_(length) { }


template <typename T>
T list<T>::operator[](uint32_t index) {
  ASSERT(index < length());
  return elms_[index];
}

template <typename T>
list<T> list<T>::sublist(uint32_t start, uint32_t length) {
  if (length == 0) return list<T>(NULL, 0);
  ASSERT(start + length < length_);
  return list<T>(elms_ + start, length);
}

template <typename T>
list_buffer<T>::list_buffer() {
  data_ = new T[kInitialCapacity];
  length_ = 0;
  capacity_ = kInitialCapacity;
}

template <typename T>
list_buffer<T>::~list_buffer() {
  delete[] data();
}

template <typename T>
T &list_buffer<T>::operator[](uint32_t index) {
  ASSERT(index < length());
  return data()[index];
}

template <typename T>
void list_buffer<T>::append(T obj) {
  if (length() >= capacity_) extend_capacity();
  data()[length_++] = obj;
}

template <typename T>
list<T> list_buffer<T>::to_list() {
  T *elms = new T[length()];
  for (uint32_t i = 0; i < length(); i++)
    elms[i] = data()[i];
  return list<T>(elms, length());
}

template <typename T>
void list_buffer<T>::push(T obj) {
  append(obj);
}

template <typename T>
T list_buffer<T>::remove_last() {
  ASSERT(length() > 0);
  return data()[--length_];
}

template <typename T>
T list_buffer<T>::peek() {
  ASSERT(length() > 0);
  return data()[length_ - 1];
}

template <typename T>
T list_buffer<T>::pop() {
  return remove_last();
}

template <typename T>
void list_buffer<T>::extend_capacity() {
  uint32_t new_capacity = grow_value(capacity_);
  T *new_data = new T[new_capacity];
  for (uint32_t i = 0; i < capacity_; i++)
    new_data[i] = data()[i];
  delete[] data();
  data_ = new_data;
  capacity_ = new_capacity;
}

}

#endif // _UTILS_LIST_INL
