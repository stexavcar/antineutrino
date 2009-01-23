#ifndef _VALUE_VALUE
#define _VALUE_VALUE

#include "utils/global.h"

namespace positron {

class Data {

};

template <class C>
static inline bool is(Data *data);

template <class C>
static inline C *cast(Data *data);

class Signal : public Data {
public:
  enum Type { sSuccess, sFailure };
  inline Type type();
};

class Success : public Signal {
public:
  static inline Success *make();
};

class Failure : public Signal {
public:
  static inline Failure *make();
};

} // namespace positron

#endif // _VALUE_VALUE
