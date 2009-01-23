#ifndef _VALUE_VALUE
#define _VALUE_VALUE

#include "utils/global.h"

namespace neutrino {

class Data {

};

template <class C>
static inline bool is(Data *data);

template <class C>
static inline C *cast(Data *data);

class Signal : public Data {
public:
  enum Type { sSuccess, sInternalError };
  inline Type type();
  inline word payload();
};

class Success : public Signal {
public:
  static inline Success *make();
};

class Failure : public Signal {
};

class InternalError : public Failure {
public:
  enum Type { ieUnknown, ieSystem, ieEnvironment };
  static inline InternalError *make(Type type);
};

} // namespace neutrino

#endif // _VALUE_VALUE
