#include "plankton/plankton-inl.h"

namespace neutrino {

ServiceRegistryEntry *ServiceRegistryEntry::first_ = NULL;

p_object ServiceRegistry::lookup(string name) {
  ServiceRegistryEntry *current = ServiceRegistryEntry::first();
  while (current != NULL) {
    if (current->name() == name)
      return current->get_instance();
    current = current->prev();
  }
  return p_object();
}

p_object ServiceRegistryEntry::get_instance() {
  if (!has_instance_) {
    instance_ = alloc_();
    has_instance_ = true;
  }
  return instance_;
}

} // namespace neutrino
