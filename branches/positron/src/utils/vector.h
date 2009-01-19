#ifndef _UTILS_VECTOR
#define _UTILS_VECTOR

#include "utils/global.h"

namespace positron {

template <typename T>
class vector {
public:
  vector() : data_(NULL), length_(0) { }
  vector(T *data, word length) : data_(data), length_(length) { }
  T &operator[](word offset);
  const T &operator[](word offset) const;
  T *start() { return data_; }
  const T *start() const { return data_; }
  word length() const { return length_; }
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

template <typename T>
class pair : public embed_vector<T, 2> {
public:
  pair() : embed_vector<T, 2>() { }
  void set(const T &a, const T &b);
};

} // namespace positron

#endif // _UTILS_VECTOR
