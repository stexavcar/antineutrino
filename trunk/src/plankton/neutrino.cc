#include "plankton/neutrino.h"

namespace neutrino {

MappingObjectProxyDescriptor &MappingObjectProxy::descriptor() {
  return descriptor_;
}

IObjectProxy::method MappingObjectProxyDescriptor::get_method(
    plankton::String name) {
  const char *c_str = name.c_str();
  method_map::iterator i = methods().find(c_str);
  delete[] c_str;
  if (i == methods().end()) return NULL;
  return (*i).second;
}

} // namespace neutrino
