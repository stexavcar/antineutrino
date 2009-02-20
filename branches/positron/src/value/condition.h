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
  Data *data() { return data_; }
protected:
  maybe(Data *data) : data_(data) { }
private:
  Data *data_;
};

template <typename T = Success>
class likely : public maybe<T, FatalError> {
public:
  likely(T *success) : maybe<T, FatalError>(success) { }
  likely(FatalError *error) : maybe<T, FatalError>(error) { }
};

class boole : public maybe<Success, Signal> {
public:
  boole(likely<> other) : maybe<Success, Signal>(other.data()) { }
  boole(Success *success) : maybe<Success, Signal>(success) { }
  boole(Signal *signal) : maybe<Success, Signal>(signal) { }
};

template <typename T>
class allocation : public maybe<T, InternalError> {
public:
  allocation(T *value) : maybe<T, InternalError>(value) { }
  template <typename S>
  allocation(allocation<S> other) : maybe<T, InternalError>(other.data()) {
    TYPE_CHECK(S, T);
  }
  allocation(InternalError *error) : maybe<T, InternalError>(error) { }
};

} // namespace neutrino

#endif // _VALUE_CONDITION
