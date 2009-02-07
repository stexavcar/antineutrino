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
  vector(array<T> data, word length) : data_(data.start()), length_(length) { }
  T &operator[](word offset);
  const T &operator[](word offset) const;
  T *start() { return data_; }
  T *end() { return start() + length(); }
  T &first();
  T &last();
  bool is_empty() { return length_ == 0; }
  const T *start() const { return data_; }
  vector<T> subvector(word start = 0, word end = -1);
  vector<T> const dup() { return subvector(); }
  word length() const { return length_; }
  static vector<T> allocate(word length);
  template <class A> static vector<T> allocate(A allocator, word length);
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

template <typename F, typename S>
class pair {
public:
  pair() { }
  pair(const F &first, const S &second) : first_(first), second_(second) { }
  F &first() { return first_; }
  S &second() { return second_; }
private:
  F first_;
  S second_;
};

} // namespace neutrino

#endif // _UTILS_VECTOR
