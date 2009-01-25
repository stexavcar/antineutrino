#ifndef _PLANKTON_PLANKTON_INL
#define _PLANKTON_PLANKTON_INL

#include "plankton/plankton.h"
#include "utils/check-inl.h"
#include "utils/log.h"

namespace neutrino {
namespace plankton {

ServiceRegistryEntry::ServiceRegistryEntry(const char *name, instance_allocator alloc)
  : name_(name), alloc_(alloc), prev_(first_), has_instance_(false) {
  first_ = this;
}

} // namespace plankton
} // namespace neutrino

namespace neutrino {

template <class T>
static inline bool is(p::Value obj) {
  return obj.type() == T::kTypeTag;
}


template <class T>
static inline T cast(p::Value obj) {
  assert is<T>(obj);
  return T(obj.data(), obj.dtable());
}

} // namespace plankton

#endif // _PLANKTON_PLANKTON_INL
