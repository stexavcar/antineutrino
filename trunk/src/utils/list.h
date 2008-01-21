#ifndef _UTILS_LIST
#define _UTILS_LIST

#include "utils/types.h"

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
  list(T *elms, uint32_t length);
  uint32_t length() { return length_; }
  T operator[](uint32_t index);
  list<T> sublist(uint32_t start, uint32_t length);
  void dispose();
private:
  T *elms_;
  uint32_t length_;
};

/**
 * A locally scoped heap-allocated extensible list.
 */
template <typename T>
class list_buffer {
public:
  list_buffer();
  ~list_buffer();
  void append(T obj);
  void push(T obj);
  T remove_last();
  T pop();
  T peek();
  T *data() { return data_; }
  T &operator[](uint32_t index);

  /**
   * Creates a list containing a copy of the contents of this list
   * buffer.  It is the caller's responsibility to dispose of this
   * list.
   */
  list<T> to_list();
  
  uint32_t length() { return length_; }
private:
  void extend_capacity();
  static const uint32_t kInitialCapacity = 4;
  T *data_;
  uint32_t length_;
  uint32_t capacity_;
};

}

#endif // _UTILS_LIST
