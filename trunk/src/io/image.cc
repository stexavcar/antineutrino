#include "heap/roots.h"
#include "heap/values-inl.h"
#include "io/image-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/list-inl.h"

namespace neutrino {

// 0xFABACEAE
// root count
// root_0
// root_1
// ...
// root_k
// image size
// word_1
// word_2
// ...

Image *Image::current_ = NULL;

Image::Image(uint32_t size, uint32_t *data)
    : size_(size)
    , data_(data)
    , heap_(0) { }

// ---------------------
// --- L o a d i n g ---
// ---------------------

bool Image::initialize() {
  if (size_ < kHeaderSize) {
    Conditions::get().error_occurred("Invalid image");
    return false;
  }
  if (data_[kMagicNumberOffset] != kMagicNumber) {
    Conditions::get().error_occurred("Invalid image");
    return false;
  }
  heap_size_ = data_[kHeapSizeOffset];
  if (heap_size_ > size_) {
    Conditions::get().error_occurred("Invalid image");
    return false;
  }
  uint32_t root_count = data_[kRootCountOffset];
  if (root_count != Roots::kCount) {
    Conditions::get().error_occurred("Invalid image");
    return false;
  }
  heap_ = data_ + kHeaderSize;
  return true;
}

void Image::copy_shallow(ImageObject *obj) {
  uint32_t instance_type = obj->instance_type();
  switch (instance_type) {
    case DICTIONARY_TYPE: {
      Data *data = Runtime::current().heap().new_dictionary();
      obj->point_forward(cast<Object>(data));
      break;
    }
    default:
      UNHANDLED(InstanceType, instance_type);
      break;
  }
}

void Image::for_each_object(ObjectCallback callback) {
  uint32_t cursor = 0;
  while (cursor < heap_size()) {
    uint32_t object_ptr = ValuePointer::tag_offset_as_object(cursor);
    ImageObject *obj = image_cast<ImageObject>(ImageData::from(object_ptr));
    callback(obj);
    cursor += obj->memory_size();
  }
}

bool Image::load() {
  Image::Scope scope(*this);
  for_each_object(copy_shallow);
  return true;
}

uint32_t ImageObject::instance_type() {
  uint32_t offset = ValuePointer::offset_of(this) + ImageObject_TypeOffset;
  uint32_t value = Image::current().heap()[offset];
  ImageData *data = ImageData::from(value);
  if (is<ImageSmi>(data)) {
    return image_cast<ImageSmi>(data)->value();
  } else if (is<ImageForwardPointer>(data)) {
    Object *target = image_cast<ImageForwardPointer>(data)->target();
    return target->chlass()->instance_type();
  } else {
    UNREACHABLE();
    return 0;
  }
}

// -----------------------------
// --- O b j e c t   s i z e ---
// -----------------------------

uint32_t ImageObject::memory_size() {
  uint32_t type = instance_type();
  switch (type) {
    case DICTIONARY_TYPE:
      return ImageDictionary_Size;
    case LAMBDA_TYPE:
      return ImageLambda_Size;
    case STRING_TYPE:
      return image_cast<ImageString>(this)->string_memory_size();
    case CODE_TYPE:
      return image_cast<ImageCode>(this)->code_memory_size();
    case TUPLE_TYPE:
      return image_cast<ImageTuple>(this)->tuple_memory_size();
    default:
      UNHANDLED(InstanceType, type);
      return 0;
  }
}

uint32_t ImageString::string_memory_size() {
  return ImageString_HeaderSize + length();
}

uint32_t ImageCode::code_memory_size() {
  return ImageCode_HeaderSize + length();
}

uint32_t ImageTuple::tuple_memory_size() {
  return ImageTuple_HeaderSize + length();
}

// ---------------------------------------
// --- F o r w a r d   P o i n t e r s ---
// ---------------------------------------

void ImageObject::point_forward(Object *obj) {
  uint32_t offset = ValuePointer::offset_of(this) + ImageObject_TypeOffset;
  ImageForwardPointer *pointer = ImageForwardPointer::to(obj);
  Image::current().heap()[offset] = reinterpret_cast<uint32_t>(pointer);
}

}
