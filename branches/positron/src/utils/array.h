#ifndef _UTILS_ARRAY
#define _UTILS_ARRAY

#include "utils/global.h"

namespace positron {

template <typename T>
class array {
public:
  array(T *data, size_t length) : data_(data), length_(length) { }
private:
  T *data_;
  size_t length_;
};

template <typename T, word L>
class embed_array {
public:
  T &operator[](word offset);
  const T &operator[](word offset) const;
private:
  T base_[L];
};

} // namespace positron

#endif // _UTILS_ARRAY
