#ifndef _UTILS_LIST_INL
#define _UTILS_LIST_INL

#include "utils/checks.h"
#include "utils/list.h"
#include "utils/vector-inl.h"


namespace neutrino {

template <typename T>
list<T>::list()
    : elms_(NULL)
    , length_(0) { }

template <typename T>
void list<T>::dispose() {
  delete[] elms_;
}

template <typename T>
T &list<T>::operator[](uword index) {
  ASSERT(index < length());
  return elms_[index];
}

template <typename T>
list<T> list<T>::sublist(uword start, uword length) {
  if (length == 0) return list<T>(NULL, 0);
  ASSERT(start + length < length_);
  return list<T>(elms_ + start, length);
}

template <typename T>
vector<T> list_buffer<T>::data() {
  return NEW_VECTOR(T, data_, length_);
}

template <typename T>
list_buffer<T>::list_buffer() {
  data_ = new T[kInitialCapacity];
  length_ = 0;
  capacity_ = kInitialCapacity;
}

template <typename T>
list_buffer<T>::~list_buffer() {
  delete[] data_;
}

template <typename T>
T &list_buffer<T>::operator[](uword index) {
  ASSERT(index < length());
  return data()[index];
}

template <typename T>
void list_buffer<T>::append(T obj) {
  ensure_capacity(length_ + 1);
  data()[length_++] = obj;
}

template <typename T>
list<T> list_buffer<T>::to_list() {
  T *elms = new T[length()];
  for (uword i = 0; i < length(); i++)
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
  T result = data()[length_ - 1];
  length_--;
  return result;
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
void list_buffer<T>::ensure_capacity(uword length) {
  if (length > capacity_)
    extend_capacity(length);
}

template <typename T>
void list_buffer<T>::ensure_length(uword length) {
  ensure_capacity(length);
  length_ = length;
}

template <typename T>
void list_buffer<T>::extend_capacity(uword required) {
  uword new_capacity = grow_value(required);
  T *new_data = new T[new_capacity];
  for (uword i = 0; i < length_; i++)
    new_data[i] = data()[i];
  delete[] data_;
  data_ = new_data;
  capacity_ = new_capacity;
}

}

#endif // _UTILS_LIST_INL
