#ifndef _UTILS_VECTOR_INL
#define _UTILS_VECTOR_INL

#include "utils/checks.h"
#include "utils/array.h"

namespace neutrino {


#ifdef DEBUG

template <typename T>
array<T>::array(T *data, uword length) : data_(data), length_(length) { }

template <typename T>
array<T>::array() : data_(NULL), length_(0) { }


#else

template <typename T>
array<T>::array(T *data) : data_(data) { }

template <typename T>
array<T>::array() : data_(NULL) { }

#endif


template <typename T>
T &array<T>::operator[](uword index) {
  ASSERT_LT_C(cnOutOfBounds, index, length_);
  return data_[index];
}


template <typename T>
uword array<T>::offset_of(T *ptr) {
  ASSERT(data_ <= ptr && ptr < (data_ + length_));
  return ptr - data_;
}


template <typename T>
T *array<T>::from_offset(uword offset) {
  ASSERT(offset < length_);
  return data_ + offset;
}


} // neutrino


#endif // _UTILS_ARRAY_INL
