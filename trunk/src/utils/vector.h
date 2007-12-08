#ifndef _UTILS_VECTOR
#define _UTILS_VECTOR

#include "utils/globals.h"

namespace neutrino {

/**
 * A vector is a wrapper around an array that bounds checks in debug
 * mode.  In product mode using a vector should be completely
 * equivalent to using a raw array.
 */
template <typename T>
class vector {
public:
  IF_DEBUG(inline vector(T *data, uint32_t length));
  IF_NO_DEBUG(inline vector(T *data));
  inline T &operator[](uint32_t index);
private:
  T *data_;
  IF_DEBUG(uint32_t length_);
};

#ifdef DEBUG
#define NEW_VECTOR(Type, data, length) vector<Type>(data, length)
#else
#define NEW_VECTOR(Type, data, length) vector<Type>(data)
#endif

}

#endif // _UTILS_ARRAY
