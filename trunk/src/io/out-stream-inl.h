#ifndef _IO_OUT_STREAM_INL
#define _IO_OUT_STREAM_INL

#include "io/out-stream.h"

namespace neutrino {

template <class C>
C *heap_buffer::allocate(InstanceType type, uword size) {
  uword offset = length();
  ensure_length(length() + size);
  address result = reinterpret_cast<address>(start() + offset);
  FObject *obj = reinterpret_cast<FObject*>(ValuePointer::tag_as_object(result));
  obj->set_header(FData::from(Smi::from_int(type)));
  return image_raw_cast<C>(obj);
}

} // neutrino

#endif // _IO_OUT_STREAM_INL
