#ifndef _VALUE_POINTER_INL
#define _VALUE_POINTER_INL

#include "value/value.h"
#include "utils/check.h"

#include "utils/log.h"
#include "utils/string-inl.h"

namespace neutrino {

/* --- S i g n a l --- */

Signal *Pointer::tag_signal(word type, word payload) {
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

word Pointer::signal_type(Signal *that) {
  uword value = reinterpret_cast<uword>(that);
  return value >> (kSignalPayloadSize + kTagSize);
}

word Pointer::signal_payload(Signal *that) {
  uword value = reinterpret_cast<uword>(that);
  return (value >> kTagSize) & ((1 << kSignalPayloadSize) - 1);
}

/* --- T a g g e d   I n t e g e r --- */

bool Pointer::fits_tagged_integer(word value) {
  static const uword kAdded = IF_ELSE(cc32, 0x40000000U, 0x2000000000000000U);
  static const uword kMask = IF_ELSE(cc32, 0x80000000U, 0x8000000000000000U);
  return ((static_cast<uword>(value) + kAdded) & kMask) == 0;
}

TaggedInteger *Pointer::tag_integer(word value) {
  assert fits_tagged_integer(value);
  TaggedInteger *result = reinterpret_cast<TaggedInteger*>(value << kTagSize | kIntegerTag);
  assert is_tagged_integer(result);
  return result;
}

bool Pointer::is_tagged_integer(Data *data) {
  word value = reinterpret_cast<word>(data);
  return (value & kTagMask) == kIntegerTag;
}

word Pointer::tagged_integer_value(TaggedInteger *that) {
  return reinterpret_cast<word>(that) >> kTagSize;
}

/* --- F o r w a r d   P o i n t e r --- */

bool Pointer::is_forward_pointer(Data *that) {
  word value = reinterpret_cast<word>(that);
  return (value & kTagMask) == kForwardPointerTag;
}

Object *Pointer::forward_pointer_target(ForwardPointer *that) {
  word value = reinterpret_cast<word>(that);
  Object *result = reinterpret_cast<Object*>(value & ~kTagMask);
  assert is_object(result);
  return result;
}

ForwardPointer *Pointer::tag_forward_pointer(Object *that) {
  word value = reinterpret_cast<word>(that);
  ForwardPointer *result = reinterpret_cast<ForwardPointer*>(value | kForwardPointerTag);
  assert is_forward_pointer(result);
  return result;
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
