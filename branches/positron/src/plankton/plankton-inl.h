#ifndef _PLANKTON_PLANKTON_INL
#define _PLANKTON_PLANKTON_INL

#include "plankton/plankton.h"
#include "utils/check-inl.h"

namespace neutrino {


template <class T>
static inline bool is(p_value obj) {
  return obj.type() == T::kTypeTag;
}


template <class T>
static inline T cast(p_value obj) {
  assert is<T>(obj);
  return T(obj.data(), obj.dtable());
}

} // namespace neutrino


#endif // _PLANKTON_PLANKTON_INL
