#ifndef _PLATFORM_DYLIB
#define _PLATFORM_DYLIB

#include "utils/string.h"

namespace neutrino {

class DynamicLibraryCollection {
public:
  virtual ~DynamicLibraryCollection() { };
  virtual bool load(string path) = 0;
  virtual void *lookup(string name) = 0;

  static DynamicLibraryCollection &create();
};

} // neutrino

#endif // _PLATFORM_DYLIB
