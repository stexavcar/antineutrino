#ifndef _POINTER_INL
#define _POINTER_INL

#include "heap/pointer.h"
#include "utils/checks.h"

namespace neutrino {

address ValuePointer::address_of(void *obj) {
  ASSERT(has_object_tag(obj));
  return reinterpret_cast<address>(reinterpret_cast<word>(obj) & ~kObjectTagMask);
}

uword ValuePointer::offset_of(void *obj) {
  STATIC_CHECK(kWordSize >= (1 << kObjectTagSize));
  ASSERT(has_object_tag(obj));
  return reinterpret_cast<word>(obj) >> kObjectTagSize;
}

Object *ValuePointer::tag_as_object(address addr) {
  ASSERT_EQ(0, reinterpret_cast<word>(addr) & kObjectTagMask);
  return reinterpret_cast<Object*>(reinterpret_cast<word>(addr) + kObjectTag);
}

uword ValuePointer::tag_offset_as_object(uword value) {
  return (value << kObjectTagSize) + kObjectTag;
}

bool ValuePointer::has_object_tag(void *val) {
  STATIC_CHECK(sizeof(void*) == sizeof(word));
  return (reinterpret_cast<word>(val) & kObjectTagMask) == kObjectTag;
}

bool ValuePointer::has_smi_tag(void *val) {
  STATIC_CHECK(sizeof(void*) == sizeof(word));
  return (reinterpret_cast<word>(val) & kSmiTagMask) == kSmiTag;
}

Smi *ValuePointer::tag_as_smi(word val) {
  STATIC_CHECK(sizeof(Smi*) == sizeof(word));
  return reinterpret_cast<Smi*>((val << kSmiTagSize) | kSmiTag);
}

word ValuePointer::value_of(void *val) {
  ASSERT(has_smi_tag(val));
  return reinterpret_cast<word>(val) >> kSmiTagSize;
}

bool ValuePointer::has_signal_tag(void *val) {
  return (reinterpret_cast<word>(val) & kObjectTagMask) == kSignalTag;
}

uword ValuePointer::tag_as_signal(address addr) {
  ASSERT_EQ(0, reinterpret_cast<word>(addr) & kObjectTagMask);
  return reinterpret_cast<word>(addr) | kSignalTag;
}

uword ValuePointer::un_signal_tag(void *value) {
  ASSERT(has_signal_tag(value));
  return reinterpret_cast<word>(value) & ~kObjectTagMask;
}

Signal *ValuePointer::tag_as_signal(uword type, uword payload) {
  ASSERT_EQ(0, type & ~kSignalTypeMask);
  ASSERT_EQ(0, payload & ~kSignalPayloadMask);
  word value = (payload << (kObjectTagSize + kSignalTypeSize))
             | (type << kObjectTagSize)
             | kSignalTag;
  return reinterpret_cast<Signal*>(value);
}

uword ValuePointer::signal_type(Signal *val) {
  word value = reinterpret_cast<word>(val);
  return (value >> kObjectTagSize) & kSignalTypeMask;
}

uword ValuePointer::signal_payload(Signal *val) {
  word value = reinterpret_cast<word>(val);
  return (value >> (kObjectTagSize + kSignalTypeSize)) & kSignalPayloadMask;
}

bool ValuePointer::is_aligned(uword size) {
  return (size & kObjectAlignmentMask) == 0;
}

uword ValuePointer::align(uword size) {
  return (size + kObjectAlignmentMask) & ~kObjectAlignmentMask;
}

template <typename T>
T &ValuePointer::access_field(Object *obj, uword offset) {
  ASSERT(is_aligned(offset));
  address addr = address_of(obj) + offset;
  ASSERT(*reinterpret_cast<uword*>(addr) != kUninitialized);
  return *reinterpret_cast<T*>(addr);
}

template <typename T>
void ValuePointer::set_field(Object *obj, uword offset, T value) {
  address addr = address_of(obj) + offset;
  *reinterpret_cast<T*>(addr) = value;
}

template <typename T>
T &ValuePointer::access_direct(Object *obj, uword offset) {
  return *reinterpret_cast<T*>(address_of(obj) + offset);
}

}

#endif // _POINTER_INL
