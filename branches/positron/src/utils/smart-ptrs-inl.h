#ifndef _UTILS_SMART_PTRS_INL
#define _UTILS_SMART_PTRS_INL

#include "utils/check.h"
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
own_vector<T>::~own_vector() {
  delete[] value_.start();
}

template <typename T>
vector<T> own_vector<T>::release() {
  vector<T> result = value_;
  value_ = vector<T>(static_cast<T*>(NULL), 0);
  return result;
}

template <typename T, typename D>
own_resource<T, D>::own_resource(const T &t)
  : state_(rsEmpty) {
  acquire(t);
}

template <typename T, typename D>
own_resource<T, D>::own_resource()
  : state_(rsEmpty) { }

template <typename T, typename D>
own_resource<T, D>::~own_resource() {
  release();
}

template <typename T, typename D>
void own_resource<T, D>::release() {
  if (state() == rsActive) {
    D::release(this->operator*());
    Abort::unregister_resource(*this);
    state_ = rsReleased;
  }
}

template <typename T, typename D>
void own_resource<T, D>::acquire(const T &t) {
  assert state() == rsEmpty;
  state_ = rsActive;
  value_ = t;
  Abort::register_resource(*this);
}

} // neutrino

#endif // _UTILS_SMART_PTRS_INL
