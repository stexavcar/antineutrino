#ifndef _VALUES_CHANNEL
#define _VALUES_CHANNEL

#include "utils/globals.h"
#include "utils/types.h"
#include "public/neutrino.h"

namespace neutrino {

class MethodDictionaryImpl : public MethodDictionary {
public:
  MethodDictionaryImpl();
  
  template <class C> C *open(IValue *obj);
  void *close(FImmediate *value);
  IValue new_value(void *origin);
};

} // neutrino

#endif // _VALUES_CHANNEL
