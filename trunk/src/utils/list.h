#ifndef _UTILS_LIST
#define _UTILS_LIST

#include "utils/misc.h"
#include "utils/types.h"
#include "utils/vector.h"

namespace neutrino {

/**
 * A wrapper that associated a length with an array and performs
 * bounds checking on access.  A list does not own the underlying
 * array but can delete it by calling the dispose method.
 */
template <typename T>
class list {
public:
  list();
  list(T *elms, uword length) : elms_(elms), length_(length) { }
  uword length() { return length_; }
  T operator[](uword index);
  list<T> sublist(uword start, uword length);
  T *start() { return elms_; }
  T *end() { return elms_ + length_; }
  void dispose();
private:
  T *elms_;
  uword length_;
};

/**
 * A locally scoped heap-allocated extensible list.
 */
template <typename T>
class list_buffer : public nocopy {
public:
  list_buffer();
  ~list_buffer();
  void append(T obj);
  void push(T obj);
  T remove_last();
  T pop();
  T peek();
  vector<T> data();
  T &operator[](uword index);
  T *start() { return data_; }

  /**
   * Creates a list containing a copy of the contents of this list
   * buffer.  It is the caller's responsibility to dispose of this
   * list.
   */
  list<T> to_list();
  
  uword length() { return length_; }

protected:
  void ensure_length(uword length);
private:
  void ensure_capacity(uword length);
  void extend_capacity(uword required);
  static const uword kInitialCapacity = 4;
  T *data_;
  uword length_;
  uword capacity_;
};

}

#endif // _UTILS_LIST
