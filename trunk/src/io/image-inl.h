#ifndef _IO_IMAGE_INL
#define _IO_IMAGE_INL

#include "heap/pointer-inl.h"
#include "io/image.h"
#include "utils/types-inl.h"

namespace neutrino {

// ---------------------
// --- Q u e r i e s ---
// ---------------------

template <>
static inline bool is<ImageObject>(ImageData *value) {
  return ValuePointer::has_object_tag(value);
}

template <>
static inline bool is<ImageSmi>(ImageData *value) {
  return ValuePointer::has_smi_tag(value);
}

template <>
static inline bool is<ImageValue>(ImageData *value) {
  return is<ImageSmi>(value) || is<ImageObject>(value);
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
DEFINE_IMAGE_OBJECT_QUERY(Lambda, LAMBDA)
DEFINE_IMAGE_OBJECT_QUERY(Dictionary, DICTIONARY)

template <class C>
static inline C *image_cast(ImageData *val) {
  ASSERT(is<C>(val));
  return reinterpret_cast<C*>(val);
}

// ---------------------------------------
// --- F o r w a r d   P o i n t e r s ---
// ---------------------------------------

void ImageObject::point_forward(Object *obj) {
  uint32_t offset = ValuePointer::offset_of(this) + ImageObject_TypeOffset;
  ImageForwardPointer *pointer = ImageForwardPointer::to(obj);
  Image::current().heap()[offset] = reinterpret_cast<uint32_t>(pointer);
}

Value *ImageValue::forward_pointer() {
  if (is<ImageSmi>(this)) {
    return image_cast<ImageSmi>(this)->forward_pointer();
  } else {
    return image_cast<ImageObject>(this)->forward_pointer();
  }
}

Smi *ImageSmi::forward_pointer() {
  return Smi::from_int(value());
}

Object *ImageObject::forward_pointer() {
  uint32_t offset = ValuePointer::offset_of(this) + ImageObject_TypeOffset;
  ImageData *value = ImageData::from(Image::current().heap()[offset]);
  ImageForwardPointer *pointer = image_cast<ImageForwardPointer>(value);
  return pointer->target();
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
    uint32_t offset = ValuePointer::offset_of(this) + Image##Class##_##Name##Offset;\
    return Image::current().heap()[offset];                          \
  }

#define DEFINE_GETTER(T, Class, name, Name)                          \
  Image##T *Image##Class::name() {                                   \
    uint32_t offset = ValuePointer::offset_of(this) + Image##Class##_##Name##Offset;\
    ImageData *data = ImageData::from(Image::current().heap()[offset]);\
    return image_cast<Image##T>(data);                               \
  }

// --- S t r i n g ---

DEFINE_RAW_GETTER(uint32_t, String, length, Length)

uint32_t ImageString::at(uint32_t index) {
  ASSERT(index < length());
  uint32_t offset = ValuePointer::offset_of(this) + ImageString_HeaderSize + index;
  return Image::current().heap()[offset];
}

// --- T u p l e ---

DEFINE_RAW_GETTER(uint32_t, Tuple, length, Length)

ImageValue *ImageTuple::at(uint32_t index) {
  ASSERT(index < length());
  uint32_t offset = ValuePointer::offset_of(this) + ImageCode_HeaderSize + index;
  ImageData *data = ImageData::from(Image::current().heap()[offset]);
  return image_cast<ImageValue>(data);
}

// --- C o d e ---

DEFINE_RAW_GETTER(uint32_t, Code, length, Length)

uint32_t ImageCode::at(uint32_t index) {
  ASSERT(index < length());
  uint32_t offset = ValuePointer::offset_of(this) + ImageCode_HeaderSize + index;
  return Image::current().heap()[offset];
}

// --- L a m b d a ---

DEFINE_RAW_GETTER(uint32_t, Lambda, argc, Argc)

DEFINE_GETTER(Code, Lambda, code, Code)
DEFINE_GETTER(Tuple, Lambda, literals, Literals)

// --- D i c t i o n a r y ---

DEFINE_GETTER(Tuple, Dictionary, table, Table)

#undef DEFINE_RAW_GETTER
#undef DEFINE_GETTER

} // neutrino

#endif // _IO_IMAGE_INL
