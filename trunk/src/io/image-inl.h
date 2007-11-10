#ifndef _IO_IMAGE_INL
#define _IO_IMAGE_INL

#include "heap/pointer-inl.h"
#include "io/image.h"
#include "utils/types-inl.h"

namespace neutrino {

template <>
static inline bool is<ImageObject>(ImageData *value) {
  return ValuePointer::has_object_tag(value);
}

template <>
static inline bool is<ImageSmi>(ImageData *value) {
  return ValuePointer::has_smi_tag(value);
}

template <>
static inline bool is<ImageForwardPointer>(ImageData *data) {
  return ValuePointer::has_signal_tag(data);
}

#define DEFINE_IMAGE_OBJECT_QUERY(Class, CLASS)                      \
  template <>                                                        \
  static inline bool is<Image##Class>(ImageData *value) {            \
    return is<ImageObject>(value)                                    \
        && image_cast<ImageObject>(value)->instance_type() == CLASS##_TYPE; \
  }

DEFINE_IMAGE_OBJECT_QUERY(String, STRING)
DEFINE_IMAGE_OBJECT_QUERY(Code, CODE)
DEFINE_IMAGE_OBJECT_QUERY(Tuple, TUPLE)

template <class C>
static inline C *image_cast(ImageData *val) {
  ASSERT(is<C>(val));
  return reinterpret_cast<C*>(val);
}

Image &Image::current() {
  ASSERT(current_ != NULL);
  return *current_;
}

Image::Scope::Scope(Image &image)
    : previous_(current_) {
  current_ = &image;
}

Image::Scope::~Scope() {
  current_ = previous_;
}

ImageData *ImageData::from(uint32_t addr) {
  return reinterpret_cast<ImageData*>(addr);
}

ImageForwardPointer *ImageForwardPointer::to(Object *obj) {
  uint32_t value = ValuePointer::tag_as_signal(ValuePointer::address_of(obj));
  return reinterpret_cast<ImageForwardPointer*>(value);
}

Object *ImageForwardPointer::target() {
  return ValuePointer::tag_as_object(reinterpret_cast<address>(ValuePointer::un_signal_tag(this)));
}

int32_t ImageSmi::value() {
  return ValuePointer::value_of(this);
}

#define DEFINE_RAW_GETTER(T, Class, name, Name)                      \
  T Image##Class::name() {                                           \
    uint32_t offset = ValuePointer::offset_of(this) + Image##Class##_##Name##Offset;        \
    return Image::current().heap()[offset];                          \
  }

DEFINE_RAW_GETTER(uint32_t, String, length, Length)

DEFINE_RAW_GETTER(uint32_t, Tuple, length, Length)

DEFINE_RAW_GETTER(uint32_t, Code, length, Length)

#undef DEFINE_RAW_GETTER

} // neutrino

#endif // _IO_IMAGE_INL
