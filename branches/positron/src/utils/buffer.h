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
  T &operator[](int index);
  T *start() { return data_; }
  int length() { return length_; }
  void ensure_capacity(int length);

private:
  void extend_capacity(int required);
  static const int kInitialCapacity = 4;
  T *data_;
  int length_;
  int capacity_;

};

}

#endif // _UTILS_BUFFER
