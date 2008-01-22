#ifndef _RUNTIME_CONTEXT
#define _RUNTIME_CONTEXT

#include "values/values.h"

namespace neutrino {

#define FOR_EACH_CONTEXT_FIELD(VISIT, arg)

class Context : public Object {
public:
  static const uint32_t kSize = Object::kHeaderSize;
};

}

#endif // _RUNTIME_CONTEXT
#ifndef _RUNTIME_CONTEXT
#define _RUNTIME_CONTEXT

namespace neutrino {

#define FOR_EACH_CONTEXT_FIELD(VISIT, arg)

class Context : public Object {
public:
  static const uint32_t kSize = Object::kHeaderSize;
};

}

#endif // _RUNTIME_CONTEXT
