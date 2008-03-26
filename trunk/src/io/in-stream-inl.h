#ifndef _IO_IN_STREAM_INL
#define _IO_IN_STREAM_INL

#include "io/in-stream.h"

namespace neutrino {

ImageIterator::ImageIterator(Image &image)
    : image_(image)
    , cursor_(image.heap())
    , limit_(image.heap() + image.heap_size()) {
}

bool ImageIterator::has_next() {
  return cursor() < limit();
}

void ImageIterator::reset() {
  cursor_ = image().heap();
}

FObject *ImageIterator::next() {
  uword object_ptr = ValuePointer::tag_as_object(cursor());
  FObject *obj = image_raw_cast<FObject>(FData::from(object_ptr));
  cursor_ += obj->size_in_image();
  return obj;
}

}

#endif // _IO_IN_STREAM_INL
