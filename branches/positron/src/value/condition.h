#ifndef _VALUE_CONDITION
#define _VALUE_CONDITION

#include "value/value.h"

namespace neutrino {

template <typename S, typename F>
class option {
public:
  option(S *s) : data_(s) { }
  option(F *f) : data_(f) { }
  inline S *value() { return cast<S>(data_); }
  inline F *failure() { return cast<F>(data_); }
  inline bool has_succeeded();
  inline bool has_failed();
  Data *data() { return data_; }
protected:
  option(Data *data) : data_(data) { }
private:
  Data *data_;
};

template <class T>
class likely : public option<T, FatalError> {
public:
  likely(T *success) : option<T, FatalError>(success) { }
  likely(FatalError *error) : option<T, FatalError>(error) { }
};

class probably : public option<Success, FatalError> {
public:
  probably(Success *success) : option<Success, FatalError>(success) { }
  probably(FatalError *signal) : option<Success, FatalError>(signal) { }
};

class possibly : public option<Success, Signal> {
public:
  possibly(probably other) : option<Success, Signal>(other.data()) { }
  possibly(Success *success) : option<Success, Signal>(success) { }
  possibly(Signal *signal) : option<Success, Signal>(signal) { }
};

template <typename T>
class allocation : public option<T, InternalError> {
public:
  allocation(T *value) : option<T, InternalError>(value) { }
  template <typename S>
  allocation(allocation<S> other) : option<T, InternalError>(other.data()) {
    TYPE_CHECK(S, T);
  }
  allocation(InternalError *error) : option<T, InternalError>(error) { }
};

} // namespace neutrino

#endif // _VALUE_CONDITION
