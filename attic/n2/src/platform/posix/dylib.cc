#include <dlfcn.h>
#include <errno.h>

#include "platform/dylib.h"
#include "utils/list-inl.h"
#include "utils/log.h"
#include "utils/string-inl.pp.h"

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
  c_string c_str(path);
  library_handle handle = dlopen(*c_str, RTLD_LAZY);
  if (handle == NULL) {
    LOG().error("%s", elms(dlerror()));
    return false;
  }
  handles().push(handle);
  return true;
}

void *PosixDynamicLibraryCollection::lookup(string name) {
  c_string c_str(name);
  for (uword i = 0; i < handles().length(); i++) {
    library_handle library = handles()[i];
    void *result = dlsym(library, *c_str);
    if (result != NULL) return result;
  }
  return NULL;
}

} // neutrino
