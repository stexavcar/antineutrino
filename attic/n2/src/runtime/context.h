#ifndef _RUNTIME_CONTEXT
#define _RUNTIME_CONTEXT

#include "values/values.h"

namespace neutrino {

#define eContextFields(VISIT, arg)

class Context : public Object {
public:
  static const uword kSize = Object::kHeaderSize;
};

}

#endif // _RUNTIME_CONTEXT
