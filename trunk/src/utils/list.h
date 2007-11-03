#ifndef _UTILS_LIST
#define _UTILS_LIST

#include "utils/types.h"

namespace neutrino {

template <typename T>
class list {
public:
  list(T *elms, uint32_t length);
  uint32_t length() { return length_; }
  T operator[](uint32_t index);
private:
  T *elms_;
  uint32_t length_;
};

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
