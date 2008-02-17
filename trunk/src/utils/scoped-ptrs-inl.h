#ifndef _SCOPED_PTRS_INL
#define _SCOPED_PTRS_INL

#include "utils/scoped-ptrs.h"

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
own_vector<T>::~own_vector() {
  delete[] value_.data();
}

template <typename T>
vector<T> own_vector<T>::release() {
  vector<T> result = value_;
  value_ = NEW_VECTOR(T, NULL, 0);
  return result;
}


} // neutrino

#endif // _SCOPED_PTRS_INL
