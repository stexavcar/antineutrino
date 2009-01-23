#ifndef _CONDITION_H
#define _CONDITION_H

#include "value/condition.h"
#include "value/value-inl.h"

namespace neutrino {

template <typename S, typename F>
bool maybe<S, F>::has_succeeded() {
  return is<S>(data_);
}

} // namespace neutrino

#endif // _CONDITION_H
