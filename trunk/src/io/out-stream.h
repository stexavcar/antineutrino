#ifndef _IO_OUT_STREAM
#define _IO_OUT_STREAM

#include "utils/list-inl.h"
#include "values/values.h"

namespace neutrino {


/**
 * An opaque datatype used to identify pointers that have been
 * serialized but not yet converted to real frozen heap pointers.
 */
struct RawFrozenValue;


class ImageOutputStream {
public:
  RawFrozenValue *marshal(Immediate *value);
  vector<uword> data() { return buffer().data(); }
private:
  list_buffer<uword> &buffer() { return buffer_; }
  list_buffer<uword> buffer_;
};


class FrozenHeap {
public:
  FrozenHeap(ImageOutputStream &stream);
  ImageValue *cook(RawFrozenValue *obj);
private:
  vector<uword> data_;
};


} // neutrino

#endif // _IO_OUT_STREAM
