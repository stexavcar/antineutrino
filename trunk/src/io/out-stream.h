#ifndef _IO_OUT_STREAM
#define _IO_OUT_STREAM

#include "utils/list-inl.h"
#include "values/values.h"

namespace neutrino {


class ImageOutputStream {
public:
  RawFValue *marshal(Immediate *value);
  vector<uword> data() { return buffer().data(); }
private:
  list_buffer<uword> &buffer() { return buffer_; }
  list_buffer<uword> buffer_;
};


class FrozenHeap {
public:
  FrozenHeap(ImageOutputStream &stream);
  FImmediate *cook(RawFValue *obj);
private:
  vector<uword> data_;
};


} // neutrino

#endif // _IO_OUT_STREAM
