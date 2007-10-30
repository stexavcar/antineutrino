#ifndef _POINTER_INL
#define _POINTER_INL

#include "heap/pointer.h"

namespace neutrino {

address ValuePointer::address_of(Object *obj) {
  ASSERT(has_object_tag(obj));
  return reinterpret_cast<address>(reinterpret_cast<word>(obj) & ~kObjectTagMask);
}

Object *ValuePointer::tag_as_object(address addr) {
  ASSERT_EQ(0, reinterpret_cast<word>(addr) & kObjectTagMask);
  return reinterpret_cast<Object*>(reinterpret_cast<word>(addr) + kObjectTag);
}

bool ValuePointer::has_object_tag(Data *val) {
  return (reinterpret_cast<word>(val) & kObjectTagMask) == kObjectTag;
}

bool ValuePointer::has_smi_tag(Data *val) {
  return (reinterpret_cast<word>(val) & kSmiTagMask) == kSmiTag;
}

Smi *ValuePointer::tag_as_smi(int32_t val) {
  STATIC_CHECK(sizeof(Smi*) == sizeof(int32_t));
  return reinterpret_cast<Smi*>((val << kSmiTagSize) | kSmiTag);
}

int32_t ValuePointer::value_of(Smi *val) {
  ASSERT(has_smi_tag(val));
  return reinterpret_cast<int32_t>(val) >> kSmiTagSize;
}

bool ValuePointer::has_signal_tag(Data *val) {
  return (reinterpret_cast<word>(val) & kObjectTagMask) == kSignalTag;
}

Signal *ValuePointer::tag_as_signal(uint32_t type, uint32_t payload) {
  ASSERT_EQ(0, type & ~kSignalTypeMask);
  ASSERT_EQ(0, payload & ~kSignalPayloadMask);
  word value = (payload << (kObjectTagSize + kSignalTypeSize))
             | (type << kObjectTagSize)
             | kSignalTag;
  return reinterpret_cast<Signal*>(value);
}

uint32_t ValuePointer::signal_type(Signal *val) {
  word value = reinterpret_cast<word>(val);
  return (value >> kObjectTagSize) & kSignalTypeMask;
}

uint32_t ValuePointer::signal_payload(Signal *val) {
  word value = reinterpret_cast<word>(val);
  return (value >> (kObjectTagSize + kSignalTypeSize)) & kSignalPayloadMask;
}

bool ValuePointer::is_aligned(uint32_t size) {
  return (size & kObjectAlignmentMask) == 0;
}

uint32_t ValuePointer::align(uint32_t size) {
  return (size + kObjectAlignmentMask) & ~kObjectAlignmentMask;
}

/**
 * Accesses the field at the specified offset, viewed as the
 * specified type.  The offset is counted in bytes and is not affected
 * by the type under which the result is viewed.
 */
template <typename T>
T &ValuePointer::access_field(Object *obj, uint32_t offset) {
  return *reinterpret_cast<T*>(address_of(obj) + offset);
}

}

#endif // _POINTER_INL
