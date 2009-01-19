#ifndef _UTILS_SMART_PTRS
#define _UTILS_SMART_PTRS

#include "utils/global.h"
#include "utils/vector.h"

namespace positron {


template <typename T>
class ptr_delete {
public:
  static void dispose(T *var) { delete var; }
};


class ptr_free {
public:
  static void dispose(void *var) { free(var); }
};


/**
 * A utility class that takes ownership of a pointer within a local
 * scope if stack-allocated within that scope.  Once the local scope
 * is exited the pointer is deallocated. An own pointer can be asked
 * to release its ownership by calling release.
 */
template < typename T, class D = ptr_delete<T> >
class own_ptr : public nocopy {
public:
  own_ptr() : value_(NULL) { }
  own_ptr(T *value) : value_(value) { }
  ~own_ptr();

  /**
   * Returns the pointer owned by this class, while letting the class
   * retain ownership.
   */
  inline T *operator *() { return value_; }
  inline void set(T *t) { value_ = t; }
  inline T *release();
  inline T *operator ->() { return value_; }

private:
  T *value_;
};


template <typename T>
class own_vector : public nocopy {
public:
  own_vector(vector<T> value) : value_(value) { }
  ~own_vector();

  inline T *start() { return value_.start(); }
  inline T &operator[](word index) { return value_[index]; }
  inline vector<T> release();
private:
  vector<T> value_;
};


} // neutrino

#endif // _UTILS_SMART_PTRS
