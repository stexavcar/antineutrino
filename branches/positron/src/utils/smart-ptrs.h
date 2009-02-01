#ifndef _UTILS_SMART_PTRS
#define _UTILS_SMART_PTRS

#include "utils/global.h"
#include "utils/vector.h"

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
  own_ptr() : value_(NULL) { }
  own_ptr(T *value) : value_(value) { }
  ~own_ptr();

  /**
   * Returns the pointer owned by this class, while letting the class
   * retain ownership.
   */
  inline T *operator *() { return value_; }
  inline void set(T *t) { value_ = t; }
  inline bool is_set() { return value_ != NULL; }
  inline T *release();
  inline T *operator ->() { return value_; }

private:
  T *value_;
};

class abstract_resource {
public:
  abstract_resource() : prev_(NULL), next_(NULL) { }
  virtual void release() = 0;
private:
  friend class Abort;
  abstract_resource *prev_;
  abstract_resource *next_;
};

enum ResourceState { rsEmpty, rsActive, rsReleased };

template <typename T, class D>
class own_resource : public abstract_resource, public nocopy {
public:
  inline ~own_resource();
  inline own_resource(const T &t);
  inline own_resource();
  void acquire(const T &t);
  const T &operator*() { return value_; }
  virtual void release();
  ResourceState state() { return state_; }
private:
  T value_;
  ResourceState state_;
};


template <>
struct coerce<ResourceState> { typedef word type; };


template <typename T>
class own_vector : public nocopy {
public:
  own_vector() { }
  own_vector(vector<T> value) : value_(value) { }
  ~own_vector();

  inline T *start() { return value_.start(); }
  inline T &operator[](word index) { return value_[index]; }
  word length() { return value_.length(); }
  inline vector<T> release();
  inline void set(vector<T> v) { value_ = v; }
  inline vector<T> as_vector() { return value_; }
private:
  vector<T> value_;
};


} // neutrino

#endif // _UTILS_SMART_PTRS
