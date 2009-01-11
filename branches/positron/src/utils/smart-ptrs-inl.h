#ifndef _UTILS_SMART_PTRS_INL
#define _UTILS_SMART_PTRS_INL

#include "utils/check.h"
#include "utils/smart-ptrs.h"

namespace positron {

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


} // positron

#endif // _UTILS_SMART_PTRS_INL
