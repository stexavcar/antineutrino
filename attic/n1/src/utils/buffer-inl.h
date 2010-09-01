#ifndef _UTILS_BUFFER_INL
#define _UTILS_BUFFER_INL

#include "utils/array-inl.h"
#include "utils/buffer.h"
#include "utils/check.h"

namespace neutrino {

template <typename T, class A>
buffer<T, A>::buffer(A allocator)
  : length_(0)
  , capacity_(kInitialCapacity)
  , allocator_(allocator) {
  data_ = allocator.allocate(kInitialCapacity);
}

template <typename T, class A>
buffer<T, A>::~buffer() {
  allocator().dispose(data_);
}

template <typename T, class A>
T &buffer<T, A>::operator[](word index) {
  assert index < capacity_;
  return start()[index];
}

template <typename T, class A>
void buffer<T, A>::append(const T &obj) {
  ensure_capacity(length_ + 1);
  start()[length_++] = obj;
}

template <typename T, class A>
void buffer<T, A>::push(const T &obj) {
  append(obj);
}

template <typename T, class A>
T buffer<T, A>::remove_last() {
  assert length() > 0;
  T result = start()[length_ - 1];
  length_--;
  return result;
}

template <typename T, class A>
const T &buffer<T, A>::peek() {
  assert length() > 0;
  return start()[length_ - 1];
}

template <typename T, class A>
T buffer<T, A>::pop() {
  return remove_last();
}

template <typename T, class A>
array<T> buffer<T, A>::allocate(word size) {
  ensure_capacity(length_ + size);
  array<T> result = TO_ARRAY(T, start() + length_, size);
  length_ += size;
  return result;
}

template <typename T, class A>
void buffer<T, A>::ensure_capacity(word length) {
  if (length > capacity_)
    extend_capacity(length);
}

template <typename T, class A>
void buffer<T, A>::extend_capacity(word required) {
  word new_capacity = grow_value(required);
  array<T> new_data = allocator().allocate(new_capacity);
  for (word i = 0; i < length_; i++)
    new_data[i] = start()[i];
  allocator().dispose(data_);
  data_ = new_data;
  capacity_ = new_capacity;
}

template <typename T, class A>
array<T> buffer<T, A>::raw_data() {
  return TO_ARRAY(T, start(), length());
}

template <typename T, class A>
vector<T> buffer<T, A>::as_vector() {
  return vector<T>(start(), length());
}

template <typename T, class D>
own_buffer<T, D>::~own_buffer() {
  for (word i = 0; i < this->length(); i++)
    D::dispose(this->operator[](i));
}

} // namespace neutrino

#endif // _UTILS_BUFFER_INL
