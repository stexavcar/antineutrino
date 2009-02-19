#ifndef _VALUE_CONDITION
#define _VALUE_CONDITION

#include "value/value.h"

namespace neutrino {

template <typename S, typename F>
class maybe {
public:
  maybe(S *s) : data_(s) { }
  maybe(F *f) : data_(f) { }
  inline S *value() { return cast<S>(data_); }
  inline F *failure() { return cast<F>(data_); }
  inline bool has_succeeded();
  inline bool has_failed();
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

class likely : public maybe<Success, FatalError> {
public:
  likely(Success *success) : maybe<Success, FatalError>(success) { }
  likely(FatalError *error) : maybe<Success, FatalError>(error) { }
};

template <typename T>
class allocation : public maybe<T, InternalError> {
public:
  allocation(T *value) : maybe<T, InternalError>(value) { }
  allocation(InternalError *error) : maybe<T, InternalError>(error) { }
};

} // namespace neutrino

#endif // _VALUE_CONDITION
