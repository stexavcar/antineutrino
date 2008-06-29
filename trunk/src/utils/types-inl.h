#ifndef _TYPES_INL
#define _TYPES_INL

#include "heap/ref-inl.h"
#include "utils/types.h"
#include "utils/checks.h"

namespace neutrino {

template <typename To, typename From>
inline To *pointer_cast(From *val) {
  TYPE_CHECK(To, From);
  return reinterpret_cast<To*>(val);
}

template <class C>
inline bool is(Data *val);

template <class T>
inline bool is(ref<Value> val) { return is<T>(*val); }

#ifdef DEBUG
template <class C> inline bool gc_safe_is(Data *val);
#endif

template <class C>
inline Option<C> to(Value *obj);

}

#endif // _TYPES_INL
