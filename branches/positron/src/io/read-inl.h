#ifndef _IO_READ_INL
#define _IO_READ_INL

#include "io/read.h"
#include "utils/check.h"

namespace neutrino {

template <typename T>
static inline bool is(s_exp *exp) {
  return exp->type() == T::kTypeTag;
}

template <typename T>
static inline T *cast(s_exp *exp) {
  assert is<T>(exp);
  return reinterpret_cast<T*>(exp);
}

} // namespace neutrino

#endif // _IO_READ_INL
