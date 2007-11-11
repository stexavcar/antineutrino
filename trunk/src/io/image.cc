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
  heap_ = data_ + kHeaderSize;
  return true;
}

Tuple *Image::load() {
  Image::Scope scope(*this);
  for_each_object(copy_object_shallow);
  for_each_object(fixup_shallow_object);
  ImageTuple *roots_img = image_cast<ImageTuple>(ImageValue::from(data_[kRootsOffset]));
  return cast<Tuple>(roots_img->forward_pointer());
}

void Image::copy_object_shallow(ImageObject *obj) {
  uint32_t instance_type = obj->instance_type();
  switch (instance_type) {
    case DICTIONARY_TYPE: {
      Dictionary *dict = cast<Dictionary>(Runtime::current().heap().new_dictionary());
      obj->point_forward(dict);
      break;
    }
    case STRING_TYPE: {
      ImageString *img = image_cast<ImageString>(obj);
      uint32_t length = img->length();
      String *str = cast<String>(Runtime::current().heap().new_string(length));
      for (uint32_t i = 0; i < length; i++)
        str->at(i) = img->at(i);
      obj->point_forward(str);
      break;
    }
    case CODE_TYPE: {
      ImageCode *img = image_cast<ImageCode>(obj);
      uint32_t length = img->length();
      Code *code = cast<Code>(Runtime::current().heap().new_code(length));
      for (uint32_t i = 0; i < length; i++)
        code->at(i) = img->at(i);
      obj->point_forward(code);
      break;
    }
    case TUPLE_TYPE: {
      ImageTuple *img = image_cast<ImageTuple>(obj);
      uint32_t length = img->length();
      Tuple *tuple = cast<Tuple>(Runtime::current().heap().new_tuple(length));
      obj->point_forward(tuple);
      break;
    }
    case LAMBDA_TYPE: {
      ImageLambda *img = image_cast<ImageLambda>(obj);
      uint32_t argc = img->argc();
      Lambda *lambda = cast<Lambda>(Runtime::current().heap().new_lambda(argc));
      obj->point_forward(lambda);
      break;
    }
    default:
      UNHANDLED(InstanceType, instance_type);
      break;
  }
}

void Image::fixup_shallow_object(ImageObject *obj) {
  uint32_t instance_type = obj->instance_type();
  switch (instance_type) {
    case DICTIONARY_TYPE: {
      ImageDictionary *img = image_cast<ImageDictionary>(obj);
      Dictionary *dict = cast<Dictionary>(img->forward_pointer());
      dict->set_table(cast<Tuple>(img->table()->forward_pointer()));
      break;
    }
    case TUPLE_TYPE: {
      ImageTuple *img = image_cast<ImageTuple>(obj);
      Tuple *tuple = cast<Tuple>(img->forward_pointer());
      uint32_t length = tuple->length();
      for (uint32_t i = 0; i < length; i++)
        tuple->at(i) = img->at(i)->forward_pointer();
      break;
    }
    case LAMBDA_TYPE: {
      ImageLambda *img = image_cast<ImageLambda>(obj);
      Lambda *lambda = cast<Lambda>(img->forward_pointer());
      lambda->set_code(cast<Code>(img->code()->forward_pointer()));
      lambda->set_literals(cast<Tuple>(img->literals()->forward_pointer()));
      break;
    }
    case STRING_TYPE: case CODE_TYPE:
      // Nothing to fix
      break;
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

}
