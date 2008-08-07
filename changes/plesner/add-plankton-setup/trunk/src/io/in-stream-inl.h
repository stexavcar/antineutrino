#ifndef _IO_IN_STREAM_INL
#define _IO_IN_STREAM_INL

#include "io/in-stream.h"

namespace neutrino {

template <class P, class D>
ImageIterator<P, D>::ImageIterator(D &heap)
    : heap_(heap)
    , cursor_(0) {
}

template <class P, class D>
bool ImageIterator<P, D>::has_next() {
  return P::has_more(heap_, cursor_);
}

template <class P, class D>
void ImageIterator<P, D>::reset() {
  cursor_ = 0;
}

template <class P, class D>
FObject *ImageIterator<P, D>::next() {
  word *addr = P::address(heap_, cursor_);
  uword object_ptr = ValuePointer::tag_as_object(addr);
  FObject *obj = image_raw_cast<FObject>(FData::from(object_ptr));
  cursor_ += obj->size_in_image();
  return obj;
}

word *FixedHeap::address(FixedHeap::Data &data, uword cursor) {
  return data.start() + cursor;
}
  
bool FixedHeap::has_more(FixedHeap::Data &data, uword cursor) {
  return cursor < data.length();
}
  
word *ExtensibleHeap::address(ExtensibleHeap::Data &data, uword cursor) {
  return data.start() + cursor;
}
  
bool ExtensibleHeap::has_more(ExtensibleHeap::Data &data, uword cursor) {
  return cursor < data.length();
}

} // neutrino

#endif // _IO_IN_STREAM_INL
