#ifndef _VALUE_CONDITION
#define _VALUE_CONDITION

#include "value/value.h"

namespace positron {

template <typename S, typename F>
class maybe {
public:
  maybe(S *s) : data_(s) { }
  maybe(F *f) : data_(f) { }
  inline bool has_succeeded();
protected:
  maybe(Data *data) : data_(data) { }
private:
  Data *data_;
};

class boole : public maybe<Success, Signal> {
public:
  boole(Success *success) : maybe<Success, Signal>(success) { }
  boole(Signal *signal) : maybe<Success, Signal>(signal) { }
};

} // namespace positron

#endif // _VALUE_CONDITION
