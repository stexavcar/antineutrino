#include "heap/values-inl.h"
#include "io/image-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/list-inl.h"

namespace neutrino {

Image::Image(uint32_t size, uint32_t *data)
    : size_(size)
    , data_(data)
    , image_(0) { }

bool Image::initialize() {
  if (size_ == 0 && data_[0] != kMagicNumber)
    return false;
  image_size_ = data_[1];
  image_ = data_ + kHeaderSize;
  ImageValue *value = ImageValue::from_addr(image_[0]);
  printf("%i\n", is<ImageObject>(value));
  printf("%i\n", is<ImageSmi>(value));
  return true;
}

}
