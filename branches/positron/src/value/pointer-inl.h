#ifndef _VALUE_POINTER_INL
#define _VALUE_POINTER_INL

#include "value/pointer.h"
#include "utils/check.h"

namespace neutrino {

Signal *Pointer::tag_signal(Signal::Type type, word payload) {
  assert payload < (1 << kSignalPayloadSize);
  uword result = (type << (kSignalPayloadSize + kTagSize))
               | (payload << kTagSize)
               | kSignalTag;
  return reinterpret_cast<Signal*>(result);
}

bool Pointer::is_signal(Data *that) {
  uword value = reinterpret_cast<uword>(that);
  return (value & kTagMask) == kSignalTag;
}

Signal::Type Pointer::signal_type(Signal *that) {
  uword value = reinterpret_cast<uword>(that);
  return static_cast<Signal::Type>(value >> (kSignalPayloadSize + kTagSize));
}

word Pointer::signal_payload(Signal *that) {
  uword value = reinterpret_cast<uword>(that);
  return (value >> kTagSize) & ((1 << kSignalPayloadSize) - 1);
}

} // namespace neutrino

#endif // _VALUE_POINTER_INL
