#ifndef _UTILS_SMART_PTRS
#define _UTILS_SMART_PTRS

#include "utils/misc.h"
#include "utils/array.h"

namespace neutrino {


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
  own_ptr(T *value) : value_(value) { }
  ~own_ptr();

  /**
   * Returns the pointer owned by this class, while letting the class
   * retain ownership.
   */
  inline T *operator *() { return value_; }
  inline T *release();
  inline T *operator ->() { return value_; }

private:
  T *value_;
};


template <typename T>
class own_array : public nocopy {
public:
  own_array(array<T> value) : value_(value) { }
  ~own_array();

  inline T *start() { return value_.start(); }
  inline T &operator[](uword index) { return value_[index]; }
  inline array<T> release();
private:
  array<T> value_;
};


/**
 * A bounded pointer is a pointer coupled with a set of bounds.  Any
 * operations performed on the pointer are asserted to stay within
 * these bounds.  In non-paranoid mode a checked pointer is just an
 * ordinary pointer.
 */
template <typename T>
class bounded_ptr {
public:
  IF_PARANOID(inline bounded_ptr(T *value, T *lower, T *upper));
  IF_NOT_PARANOID(inline bounded_ptr(T *value));
  inline T &operator[](uword index);
  inline T &operator*();
  inline bounded_ptr<T> operator+(word delta);
  inline bounded_ptr<T> operator-(word delta);
  inline bool operator>=(bounded_ptr<T> that) { return value_ >= that.value_; }
  inline word operator-(bounded_ptr<T> that);
  inline T *value() { return value_; }
private:
  T *value_;
  IF_PARANOID(T *lower_);
  IF_PARANOID(T *upper_);
};


#ifdef PARANOID

/**
 * This function is used to get argument type inference.
 */
template <typename T>
static inline bounded_ptr<T> new_bounded_ptr(T *value, T *lower, T *upper) {
  return bounded_ptr<T>(value, lower, upper);
}

#define NEW_BOUNDED_PTR(value, lower, upper) neutrino::new_bounded_ptr(value, lower, upper)

#else

/**
 * This function is used to get argument type inference.
 */
template <typename T>
static inline bounded_ptr<T> new_bounded_ptr(T *value) {
  return bounded_ptr<T>(value);
}

#define NEW_BOUNDED_PTR(value, upper, lower) neutrino::new_bounded_ptr(value)

#endif


} // neutrino

#endif // _UTILS_SMART_PTRS