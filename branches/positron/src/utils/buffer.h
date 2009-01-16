#ifndef _UTILS_BUFFER
#define _UTILS_BUFFER

#include "utils/global.h"

namespace positron {

template <typename T>
class buffer : public nocopy {
public:
  inline buffer();
  inline ~buffer();
  void append(const T &obj);
  void push(const T &obj);
  T remove_last();
  T pop();
  const T &peek();
  T &operator[](size_t index);
  T *start() { return data_; }
  size_t length() { return length_; }
  void ensure_capacity(size_t length);

private:
  void extend_capacity(size_t required);
  static const word kInitialCapacity = 4;
  T *data_;
  size_t length_;
  size_t capacity_;

};

}

#endif // _UTILS_BUFFER
