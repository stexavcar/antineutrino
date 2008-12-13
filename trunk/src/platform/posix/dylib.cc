#include <dlfcn.h>
#include <errno.h>

#include "platform/dylib.h"
#include "utils/list-inl.h"
#include "utils/log.h"

namespace neutrino {

typedef void *library_handle;

class PosixDynamicLibraryCollection : public DynamicLibraryCollection {
public:
  virtual ~PosixDynamicLibraryCollection();
  virtual bool load(string path);
  virtual void *lookup(string name);
private:
  list_buffer<library_handle> &handles() { return handles_; }
  list_buffer<library_handle> handles_;
};

DynamicLibraryCollection &DynamicLibraryCollection::create() {
  return *new PosixDynamicLibraryCollection();
}

PosixDynamicLibraryCollection::~PosixDynamicLibraryCollection() {
  for (uword i = 0; i < handles().length(); i++)
    dlclose(handles()[i]);
}

bool PosixDynamicLibraryCollection::load(string path) {
  library_handle handle = dlopen(path.chars(), RTLD_LAZY);
  if (handle == NULL) {
    LOG().error("%s", elms(dlerror()));
    return false;
  }
  handles().push(handle);
  return true;
}

void *PosixDynamicLibraryCollection::lookup(string name) {
  void *result = dlsym(RTLD_SELF, name.chars());
  if (result != NULL) return result;
  for (uword i = 0; i < handles().length(); i++) {
    library_handle library = handles()[i];
    result = dlsym(library, name.chars());
    if (result != NULL) return result;
  }
  return NULL;
}

} // neutrino
