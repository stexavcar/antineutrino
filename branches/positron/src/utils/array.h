#ifndef _UTILS_ARRAY
#define _UTILS_ARRAY

#include "utils/global.h"

namespace positron {

template <typename T>
class array {
public:
#ifdef ARRAY_BOUNDS_CHECKS
  array(T *data, word length) : data_(data), length_(length) { }
#else
  array(T *data) : data_(data) { }
#endif // ARRAY_BOUNDS_CHECKS
  T &operator[](word offset);
  const T &operator[](word offset) const;
  array<T> from(word start);
  T *start() { return data_; }
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

} // namespace positron

#endif // _UTILS_ARRAY
