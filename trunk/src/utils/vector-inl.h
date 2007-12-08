#ifndef _UTILS_VECTOR_INL
#define _UTILS_VECTOR_INL

#include "utils/vector.h"

namespace neutrino {

#ifdef DEBUG
template <typename T>
vector<T>::vector(T *data, uint32_t length) : data_(data), length_(length) { }
#else
template <typename T>
vector<T>::vector(T *data) : data_(data) { }
#endif


template <typename T>
T &vector<T>::operator[](uint32_t index) {
  ASSERT_LT(index, length_);
  return data_[index];
}

}

#endif // _UTILS_ARRAY_INL
