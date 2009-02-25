#ifndef _UTILS_ARRAY
#define _UTILS_ARRAY

#include "utils/global.h"

namespace neutrino {

template <typename T>
class array {
public:
#ifdef ARRAY_BOUNDS_CHECKS
  array(T *data, word length) : data_(data), length_(length) { }
  array() : data_(NULL), length_(0) { }
  word length() { return length_; }
#else
  array(T *data) : data_(data) { }
  array() : data_(NULL) { }
#endif // ARRAY_BOUNDS_CHECKS
  bool is_empty() { return data_ == NULL; }
  T &operator[](word offset);
  const T &operator[](word offset) const;
  array<T> from(word start);
  T *start() { return data_; }
  inline void copy_to(array<T> that, word length);
private:
  T *data_;
  IF(ccArrayBoundsChecks, word length_);
};

template <typename T, word L>
class embed_array : public array<T> {
public:
#ifdef ARRAY_BOUNDS_CHECKS
  embed_array() : array<T>(base_, L) { }
#else
  embed_array() : array<T>(base_) { }
#endif
private:
  T base_[L];
};

#ifdef ARRAY_BOUNDS_CHECKS
#define TO_ARRAY(T, data, length) array<T>(data, length)
#else
#define TO_ARRAY(T, data, length) array<T>(data)
#endif

template <typename T>
class new_delete_array_allocator {
public:
  array<T> allocate(size_t size) { return TO_ARRAY(T, new T[size], size); }
  void dispose(array<T> obj) { delete[] obj.start(); }
};

} // namespace neutrino

#endif // _UTILS_ARRAY
