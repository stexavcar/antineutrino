#ifndef _IO_IMAGE_INL
#define _IO_IMAGE_INL

#include "heap/pointer-inl.h"
#include "io/image.h"
#include "utils/types-inl.h"

namespace neutrino {

ImageValue *ImageValue::from_addr(uint32_t addr) {
  return reinterpret_cast<ImageValue*>(addr);
}

template <>
static inline bool is<ImageObject>(ImageValue *value) {
  return ValuePointer::has_object_tag(value);
}

template <>
static inline bool is<ImageSmi>(ImageValue *value) {
  return ValuePointer::has_smi_tag(value);
}

}

#endif // _IO_IMAGE_INL
