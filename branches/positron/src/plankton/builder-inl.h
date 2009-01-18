#ifndef _PLANKTON_BUILDER_INL
#define _PLANKTON_BUILDER_INL

#include "plankton/builder.h"
#include "plankton/plankton.h"
#include "utils/array-inl.h"
#include "utils/buffer.h"
#include "utils/string.h"

namespace positron {

uint32_t Raw::tag_object(word offset) {
  return static_cast<uint32_t>((offset << kTagSize) | kObjectTag);
}

word Raw::untag_object(uint32_t data) {
  assert has_object_tag(data);
  return data >> kTagSize;
}

bool Raw::has_object_tag(uint32_t data) {
  return (data & kTagMask) == kObjectTag;
}

uint32_t Raw::tag_integer(word value) {
  return static_cast<uint32_t>((value << kTagSize) | kIntegerTag);
}

word Raw::untag_integer(uint32_t data) {
  assert has_integer_tag(data);
  return data >> kTagSize;
}

bool Raw::has_integer_tag(uint32_t data) {
  return (data & kTagMask) == kIntegerTag;
}

uint32_t Raw::tag_singleton(p_value::Type value) {
  return static_cast<uint32_t>((value << kTagSize) | kSingletonTag);
}

p_value::Type Raw::untag_singleton(uint32_t data) {
  assert has_singleton_tag(data);
  return static_cast<p_value::Type>(data >> kTagSize);
}

bool Raw::has_singleton_tag(uint32_t data) {
  return (data & kTagMask) == kSingletonTag;
}

template <typename T>
T Builder::to_plankton(object &obj) {
  word offset = obj.start() - this->data().start();
  return T(Raw::tag_object(static_cast<uint32_t>(offset)), this->dtable());
}

template <typename T>
T &object::at(word offset) {
  return *reinterpret_cast<uint32_t*>(&data()[sizeof(T) * offset]);
}

} // namespace positron

#endif // _PLANKTON_BUILDER_INL
