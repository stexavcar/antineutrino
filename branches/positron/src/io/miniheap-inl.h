#ifndef _PLANKTON_BUILDER_INL
#define _PLANKTON_BUILDER_INL

#include "io/miniheap.h"
#include "plankton/plankton.h"
#include "utils/array-inl.h"
#include "utils/buffer.h"
#include "utils/string.h"

namespace neutrino {

uint32_t MiniHeapPointer::tag_object(word offset) {
  return static_cast<uint32_t>((offset << kTagSize) | kObjectTag);
}

word MiniHeapPointer::untag_object(uint32_t data) {
  assert has_object_tag(data);
  return data >> kTagSize;
}

bool MiniHeapPointer::has_object_tag(uint32_t data) {
  return (data & kTagMask) == kObjectTag;
}

uint32_t MiniHeapPointer::tag_integer(word value) {
  return static_cast<uint32_t>((value << kTagSize) | kIntegerTag);
}

word MiniHeapPointer::untag_integer(uint32_t data) {
  assert has_integer_tag(data);
  return data >> kTagSize;
}

bool MiniHeapPointer::has_integer_tag(uint32_t data) {
  return (data & kTagMask) == kIntegerTag;
}

uint32_t MiniHeapPointer::tag_singleton(p::Value::Type value) {
  return static_cast<uint32_t>((value << kTagSize) | kSingletonTag);
}

p::Value::Type MiniHeapPointer::untag_singleton(uint32_t data) {
  assert has_singleton_tag(data);
  return static_cast<p::Value::Type>(data >> kTagSize);
}

bool MiniHeapPointer::has_singleton_tag(uint32_t data) {
  return (data & kTagMask) == kSingletonTag;
}

template <typename T>
T MiniHeap::to_plankton(MiniHeapObject &obj) {
  vector<uint8_t> heap = memory();
  word offset = obj.start() - heap.start();
  return T(MiniHeapPointer::tag_object(static_cast<uint32_t>(offset)), &dtable());
}

template <typename T>
T &MiniHeapObject::at(word offset) {
  return *reinterpret_cast<uint32_t*>(&data()[sizeof(T) * offset]);
}

MessageIn::~MessageIn() {
  if (is_synchronous_ && !has_replied_)
    reply(MessageOut::get_void());
}

} // namespace neutrino

#endif // _PLANKTON_BUILDER_INL
