#ifndef _UTILS_VECTOR
#define _UTILS_VECTOR

#include "utils/global.h"

namespace positron {

template <typename T>
class vector {
public:
  vector(T *data, word length) : data_(data), length_(length) { }
  T &operator[](word offset);
  const T &operator[](word offset) const;
  T *start() { return data_; }
  word length() { return length_; }
private:
  T *data_;
  word length_;
};

template <typename T, word L>
class embed_vector : public vector<T> {
public:
  embed_vector() : vector<T>(base_, L) { }
private:
  T base_[L];
};

} // namespace positron

#endif // _UTILS_VECTOR
