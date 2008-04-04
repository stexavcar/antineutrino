#ifndef _UTILS_VECTOR_INL
#define _UTILS_VECTOR_INL

#include "utils/checks.h"
#include "utils/vector.h"

namespace neutrino {


#ifdef DEBUG

template <typename T>
vector<T>::vector(T *data, uword length) : data_(data), length_(length) { }

template <typename T>
vector<T>::vector() : data_(NULL), length_(0) { }


#else

template <typename T>
vector<T>::vector(T *data) : data_(data) { }

template <typename T>
vector<T>::vector() : data_(NULL) { }

#endif


template <typename T>
T &vector<T>::operator[](uword index) {
  ASSERT_LT_C(cnOutOfBounds, index, length_);
  return data_[index];
}

}

#endif // _UTILS_ARRAY_INL
