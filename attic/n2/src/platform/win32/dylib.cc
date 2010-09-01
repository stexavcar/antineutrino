#include "platform/dylib.h"
#include "utils/list-inl.h"

namespace win32 {

#include <windows.h>

} // win32

namespace neutrino {

class Win32DynamicLibraryCollection : public DynamicLibraryCollection {
public:
  virtual ~Win32DynamicLibraryCollection();
  virtual bool load(string path);
  virtual void *lookup(string name);
private:
  list_buffer<win32::HINSTANCE> &handles() { return handles_; }
  list_buffer<win32::HINSTANCE> handles_;
};

DynamicLibraryCollection &DynamicLibraryCollection::create() {
  return *new Win32DynamicLibraryCollection();
}

Win32DynamicLibraryCollection::~Win32DynamicLibraryCollection() {
  for (uword i = 0; i < handles().length(); i++)
    FreeLibrary(handles()[i]);
}

bool Win32DynamicLibraryCollection::load(string path) {
  win32::HINSTANCE handle = win32::LoadLibraryA(path.chars());
  if (handle == NULL) return false;
  handles().push(handle);
  return true;
}

void *Win32DynamicLibraryCollection::lookup(string name) {
  for (uword i = 0; i < handles().length(); i++) {
    win32::HINSTANCE library = handles()[i];
    void *result = GetProcAddress(library, name.chars());
    if (result != NULL) return result;
  }
  return NULL;
}

} // neutrino
