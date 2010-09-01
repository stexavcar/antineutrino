#ifndef _CONDITION_H
#define _CONDITION_H

#include "value/condition.h"
#include "value/value-inl.h"

namespace neutrino {

template <typename S, typename F>
bool option<S, F>::has_succeeded() {
  return is<S>(data_);
}

template <typename S, typename F>
bool option<S, F>::has_failed() {
  return is<F>(data_);
}

} // namespace neutrino

#endif // _CONDITION_H
