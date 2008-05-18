#ifndef _UTILS_VECTOR
#define _UTILS_VECTOR

#include "utils/globals.h"

namespace neutrino {

/**
 * An array is a wrapper around a primitive array that bounds checks
 * in debug mode.  In product mode using a array should be completely
 * equivalent to using a raw array.
 */
template <typename T>
class array {
public:
  inline array();
  IF_DEBUG(inline array(T *data, uword length));
  IF_NOT_DEBUG(inline array(T *data));
  inline T &operator[](uword index);
  inline T *start() { return data_; }
  inline T *from_offset(uword offset);
  inline uword offset_of(T *ptr);
private:
  T *data_;
  IF_DEBUG(uword length_);
};

#define ALLOCATE_ARRAY(Type, length) NEW_ARRAY(Type, new Type[length], length)

#ifdef DEBUG
#define NEW_ARRAY(Type, data, length) neutrino::array<Type>(data, length)
#else
#define NEW_ARRAY(Type, data, length) neutrino::array<Type>(data)
#endif

}

#endif // _UTILS_ARRAY
