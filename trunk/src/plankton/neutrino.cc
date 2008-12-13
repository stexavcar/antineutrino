#include "plankton/neutrino.h"

namespace neutrino {

MappingObjectProxyDescriptor &MappingObjectProxy::descriptor() {
  return descriptor_;
}

IObjectProxy::method MappingObjectProxyDescriptor::get_method(
    plankton::String name, int argc) {
  const char *c_str = name.c_str();
  method_map::iterator i = methods().find(selector(c_str, argc));
  delete[] c_str;
  if (i == methods().end()) return NULL;
  return (*i).second;
}


RegisterInternalChannel *RegisterInternalChannel::first_ = NULL;


RegisterInternalChannel::RegisterInternalChannel(const char *name,
    callback_t *callback)
  : name_(name)
  , callback_(callback)
  , prev_(first_) {
  first_ = this;
}


} // namespace neutrino
