#ifndef _UTILS_ARRAY
#define _UTILS_ARRAY

#include "utils/global.h"

namespace positron {

template <typename T>
class array {
public:
#ifdef ARRAY_BOUNDS_CHECKS
  array(T *data, size_t length) : data_(data), length_(length) { }
#else
  array(T *data) : data_(data) { }
#endif // ARRAY_BOUNDS_CHECKS
  T &operator[](size_t offset);
  const T &operator[](size_t offset) const;
  T *start() { return data_; }
private:
  T *data_;
  IF(ccArrayBoundsChecks, size_t length_);
};

template <typename T, word L>
class embed_array : public array<T> {
public:
  embed_array() : array<T>(base_, L) { }
private:
  T base_[L];
};

#ifdef ARRAY_BOUNDS_CHECKS
#define TO_ARRAY(data, length) array(data, length)
#else
#define TO_ARRAY(data, length) array(data)
#endif

} // namespace positron

#endif // _UTILS_ARRAY
