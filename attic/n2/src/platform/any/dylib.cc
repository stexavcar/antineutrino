#include "platform/dylib.h"
#include "utils/list-inl.h"

/**
 * A dummy implementation of dynamic loading that fails to load
 * anything.
 */

namespace neutrino {

class DummyDynamicLibraryCollection : public DynamicLibraryCollection {
public:
  virtual ~DummyDynamicLibraryCollection();
  virtual bool load(string path);
  virtual void *lookup(string name);
};

DynamicLibraryCollection &DynamicLibraryCollection::create() {
  static DummyDynamicLibraryCollection kInstance;
  return kInstance;
}

DummyDynamicLibraryCollection::~DummyDynamicLibraryCollection() { }

bool DummyDynamicLibraryCollection::load(string path) {
  return false;
}

void *DummyDynamicLibraryCollection::lookup(string name) {
  return NULL;
}

} // neutrino
