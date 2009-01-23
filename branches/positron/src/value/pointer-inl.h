#ifndef _VALUE_POINTER_INL
#define _VALUE_POINTER_INL

#include "value/pointer.h"

namespace positron {

Signal *Pointer::tag_signal(Signal::Type type) {
  return reinterpret_cast<Signal*>((type << kTagSize) | kSignalTag);
}

bool Pointer::is_signal(Data *data) {
  return (reinterpret_cast<word>(data) & kTagMask) == kSignalTag;
}

Signal::Type Pointer::signal_type(Signal *that) {
  return static_cast<Signal::Type>(reinterpret_cast<word>(that) >> kTagSize);
}

} // namespace positron

#endif // _VALUE_POINTER_INL
