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
  inline vector();
  IF_DEBUG(inline vector(T *data, uword length));
  IF_NOT_DEBUG(inline vector(T *data));
  inline T &operator[](uword index);
private:
  T *data_;
  IF_DEBUG(uword length_);
};

#ifdef DEBUG
#define NEW_VECTOR(Type, data, length) neutrino::vector<Type>(data, length)
#else
#define NEW_VECTOR(Type, data, length) neutrino::vector<Type>(data)
#endif

}

#endif // _UTILS_ARRAY
