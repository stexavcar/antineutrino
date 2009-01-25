#include "plankton/plankton-inl.h"
#include "utils/string-inl.h"

namespace neutrino {
namespace plankton {


Value::DTable *String::c_string_dtable_ = NULL;
ServiceRegistryEntry *ServiceRegistryEntry::first_ = NULL;

class CStringDTable : public Value::DTable {
public:
  CStringDTable() {
    string.length = string_length;
    string.compare = string_compare;
    string.get = string_get;
  }
private:
  static word string_length(String str);
  static uint32_t string_get(String str, word index);
  static word string_compare(String that, String other);
  static inline const char *open(String str);
};

const char *CStringDTable::open(String str) {
  return reinterpret_cast<const char *>(str.data());
}

word CStringDTable::string_length(String str) {
  return strlen(open(str));
}

uint32_t CStringDTable::string_get(String str, word index) {
  return open(str)[index];
}

word CStringDTable::string_compare(String that, String other) {
  for (word i = 0; true; i++) {
    uint32_t ca = that[i];
    uint32_t cb = other[i];
    if (ca == cb) {
      if (ca == '\0')
        return 0;
    } else if (ca == '\0') {
      return -1;
    } else if (cb == '\0') {
      return 1;
    } else {
      return static_cast<word>(ca) - static_cast<word>(cb);
    }
  }
}

Value::DTable *String::c_string_dtable() {
  if (c_string_dtable_ == NULL)
    c_string_dtable_ = new CStringDTable();
  return c_string_dtable_;
}

Object ServiceRegistry::lookup(String name) {
  ServiceRegistryEntry *current = ServiceRegistryEntry::first();
  while (current != NULL) {
    if (name == current->name())
      return current->get_instance();
    current = current->prev();
  }
  return Object();
}

Object ServiceRegistryEntry::get_instance() {
  if (!has_instance_) {
    instance_ = alloc_();
    has_instance_ = true;
  }
  return instance_;
}

} // namespace plankton
} // namespace neutrino
