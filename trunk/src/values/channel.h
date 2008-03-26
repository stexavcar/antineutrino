#ifndef _VALUES_CHANNEL
#define _VALUES_CHANNEL

#include "utils/globals.h"
#include "utils/types.h"
#include "public/neutrino.h"

namespace neutrino {

class MethodDictionaryImpl : public IMethodDictionary {
public:
  virtual ValueType type(IValue *that);
  virtual int value(IInteger *that);
  virtual int length(IString *that);
  virtual const char *c_str(IString *that);
  virtual int length(ITuple *that);
  virtual IValue get(ITuple *that, int index);

  template <class C> C *open(IValue *obj);
  void *close(FImmediate *value);
  IValue new_value(void *origin);
};

} // neutrino

#endif // _VALUES_CHANNEL
