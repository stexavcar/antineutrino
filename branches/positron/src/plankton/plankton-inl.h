#ifndef _PLANKTON_PLANKTON_INL
#define _PLANKTON_PLANKTON_INL

#include "plankton/plankton.h"
#include "utils/check-inl.h"
#include "utils/log.h"

namespace neutrino {
namespace plankton {

Value::Value(word value)
  : data_(value)
  , dtable_(Integer::literal_adapter()) { }

Value::Value(const char *str)
  : data_(reinterpret_cast<word>(str))
  , dtable_(String::char_ptr_adapter()) { }

LiteralArray Array::of(Value v0, Value v1, Value v2, Value v3,
    Value v4, Value v5) {
  return LiteralArray(v0, v1, v2, v3, v4, v5);
}

LiteralArray::LiteralArray(Value v0, Value v1, Value v2, Value v3,
    Value v4, Value v5)
  : Array(reinterpret_cast<word>(values_), literal_array_adapter()) {
  values_[0] = v0;
  values_[1] = v1;
  values_[2] = v2;
  values_[3] = v3;
  values_[4] = v4;
  values_[5] = v5;
}

ServiceRegistryEntry::ServiceRegistryEntry(const char *name, instance_allocator alloc)
  : name_(name), alloc_(alloc), prev_(first_), has_instance_(false) {
  first_ = this;
}

class MessageData {
public:
  MessageData();
  ~MessageData();
  void acquire_resource(IMessageResource &resource);
private:
  buffer<IMessageResource*> &resources() { return resources_; }
  own_buffer<IMessageResource> resources_;
};

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


template <class T>
static inline bool is_seed(p::Seed obj) {
  return p::Seed::belongs_to(obj, T::oid());
}


template <class T>
static inline bool is_seed(p::Value obj) {
  return is<p::Seed>(obj) && is_seed<T>(cast<p::Seed>(obj));
}

template <>
class variant_type_impl<p::Value> : public variant_type {
public:
  static inline void initialize(variant &that, p::Value value) {
    that.type_ = &kInstance;
    that.data_.u_pair.first = reinterpret_cast<void*>(value.data());
    that.data_.u_pair.second = value.dtable();
  }
  virtual void print_on(const void *data, string modifiers,
      string_stream &stream);
  static variant_type_impl<p::Value> kInstance;
};

} // namespace plankton

#endif // _PLANKTON_PLANKTON_INL
