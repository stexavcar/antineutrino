#ifndef _VALUE_POINTER_INL
#define _VALUE_POINTER_INL

#include "value/pointer.h"
#include "utils/check.h"

#include "utils/log.h"
#include "utils/string-inl.h"

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

bool Pointer::fits_small_double(double value) {
#ifdef M64
  return ((double_bits(value) - kDoubleBias) & kStolenDoubleBitsMask) == 0;
#else
  return false;
#endif // M64
}

#ifdef M64
bool Pointer::is_small_double(Data *value) {
  return (reinterpret_cast<uword>(value) & kTagMask) == kSmadTag;
}

SmallDouble *Pointer::tag_small_double(double value) {
  assert fits_small_double(value);
  static const uword kLowerMask = (static_cast<uword>(1) << kStolenDoubleBitsStart) - 1;
  static const uword kUpperMask = (static_cast<uword>(1) << 63);
  uword bits = double_bits(value) - kDoubleBias;
  uword result = (bits & kUpperMask) | ((bits & kLowerMask) << kStolenDoubleBitsCount) | kSmadTag;
  return reinterpret_cast<SmallDouble*>(result);
}

double Pointer::untag_small_double(SmallDouble *value) {
  static const uword kLowerMask = (static_cast<uword>(1) << kStolenDoubleBitsStart) - 1;
  static const uword kUpperMask = (static_cast<uword>(1) << 63);
  uword bits = reinterpret_cast<uword>(value);
  uword result = (bits & kUpperMask) | ((bits >> kStolenDoubleBitsCount) & kLowerMask);
  return make_double(result + kDoubleBias);
}
#endif

} // namespace neutrino

#endif // _VALUE_POINTER_INL
