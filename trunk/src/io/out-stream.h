#ifndef _IO_OUT_STREAM
#define _IO_OUT_STREAM

#include "utils/list-inl.h"
#include "values/values.h"

namespace neutrino {


class heap_buffer : public list_buffer<word> {
public:
  template <class C>
  C *allocate(InstanceType type, uword size);
  word *cursor() { return data().data() + length(); }
private:
};


class ImageOutputStream {
public:
  RawFValue *marshal(Immediate *value);
  RawFValue *marshal_object_shallow(Object *obj);
  vector<word> data() { return buffer().data(); }
private:
  heap_buffer &buffer() { return buffer_; }
  heap_buffer buffer_;
};


class FrozenHeap {
public:
  FrozenHeap(ImageOutputStream &stream);
  FImmediate *cook(RawFValue *obj);
private:
  vector<word> &data() { return data_; }
  vector<word> data_;
};


} // neutrino

#endif // _IO_OUT_STREAM
