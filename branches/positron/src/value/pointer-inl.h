#ifndef _VALUE_POINTER_INL
#define _VALUE_POINTER_INL

#include "value/pointer.h"
#include "utils/check.h"

namespace neutrino {

/* --- S i g n a l --- */

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

/* --- F o r w a r d   P o i n t e r --- */

bool Pointer::is_forward_pointer(Data *that) {
  word value = reinterpret_cast<word>(that);
  return (value & kTagMask) == kForwardPointerTag;
}

Object *Pointer::forward_pointer_target(ForwardPointer *that) {
  word value = reinterpret_cast<word>(that);
  return reinterpret_cast<Object*>(value & ~kTagMask);
}

ForwardPointer *Pointer::tag_forward_pointer(Object *that) {
  word value = reinterpret_cast<word>(that);
  return reinterpret_cast<ForwardPointer*>(value | kTagMask);
}

/* --- O b j e c t --- */

bool Pointer::is_object(Data *that) {
  uword value = reinterpret_cast<uword>(that);
  return (value & kTagMask) == kObjectTag;
}

Object *Pointer::as_object(uint8_t *addr) {
  return reinterpret_cast<Object*>(addr);
}

/* --- T a g g e d   D o u b l e --- */

bool Pointer::fits_tagged_double(double value) {
#ifdef M64
  union {
    double value;
    uint64_t bits;
  } u;
  u.value = value;
  return (u.bits & kStolenDoubleBitsMask) == 0;
#else
  return false;
#endif // M64
}

} // namespace neutrino

#endif // _VALUE_POINTER_INL
