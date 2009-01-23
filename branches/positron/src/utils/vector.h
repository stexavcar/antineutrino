#ifndef _UTILS_VECTOR
#define _UTILS_VECTOR

#include "utils/array.h"
#include "utils/global.h"

namespace neutrino {

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
  static vector<T> allocate(word length);
  array<T> as_array();
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

} // namespace neutrino

#endif // _UTILS_VECTOR
