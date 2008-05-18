#ifndef _UTILS_SMART_PTRS_INL
#define _UTILS_SMART_PTRS_INL

#include "utils/smart-ptrs.h"

namespace neutrino {

template <typename T, class D>
own_ptr<T, D>::~own_ptr() {
  D::dispose(value_);
}

template <typename T, class D>
inline T *own_ptr<T, D>::release() {
  T *result = value_;
  value_ = NULL;
  return result;
}

template <typename T>
own_array<T>::~own_array() {
  delete[] value_.start();
}

template <typename T>
array<T> own_array<T>::release() {
  array<T> result = value_;
  value_ = NEW_ARRAY(static_cast<T*>(NULL), 0);
  return result;
}


// -------------------------------------
// --- C h e c k e d   P o i n t e r ---
// -------------------------------------


template <typename T>
inline T &bounded_ptr<T>::operator*() {
  return this->operator[](0);
}


template <typename T>
inline T &bounded_ptr<T>::operator[](uword index) {
  IF_PARANOID(CHECK_C(cnOutOfBounds, value_ + index < upper_));
  return value_[index];
}


template <typename T>
bounded_ptr<T> bounded_ptr<T>::operator+(word d) {
  return NEW_BOUNDED_PTR(value_ + d, lower_, upper_);
}


template <typename T>
bounded_ptr<T> bounded_ptr<T>::operator-(word d) {
  return NEW_BOUNDED_PTR(value_ - d, lower_, upper_);
}


template <typename T>
word bounded_ptr<T>::operator-(bounded_ptr<T> that) {
  return value_ - that.value_;
}


#ifdef PARANOID

template <typename T>
bounded_ptr<T>::bounded_ptr(T *value, T *lower, T *upper)
    : value_(value)
    , lower_(lower)
    , upper_(upper) {
  ASSERT(lower <= value && value < upper);
}

#else

template <typename T>
bounded_ptr<T>::bounded_ptr(T *value)
    : value_(value) { }

#endif // PARANOID


} // neutrino

#endif // _UTILS_SMART_PTRS_INL
