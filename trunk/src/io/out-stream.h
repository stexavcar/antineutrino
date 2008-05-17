#ifndef _IO_OUT_STREAM
#define _IO_OUT_STREAM

#include "io/image.h"
#include "utils/list-inl.h"
#include "values/values.h"

namespace neutrino {


class heap_buffer : public list_buffer<word> {
public:
  template <class C>
  C *allocate(InstanceType type, uword size);
  word *cursor() { return data().start() + length(); }
private:
};


class Serializer {
public:
  Serializer();
  RawFValue *marshal(Immediate *value);
  RawFValue *marshal_object_shallow(Object *obj);
  void marshal_object_deep(FObject *obj);
  void flush();
  bool has_been_flushed() { return has_been_flushed_; }
  vector<word> data() { return buffer().data(); }
private:
  heap_buffer &buffer() { return buffer_; }
  heap_buffer buffer_;
  bool has_been_flushed_;
};


/**
 * A frozen heap is a chunk of memory that contains serialized
 * objects.
 */
class FrozenHeap {
public:
  FrozenHeap(Serializer &stream);
  FImmediate *cook(RawFValue *obj);
private:
  vector<word> &data() { return data_; }
  vector<word> data_;
};


} // neutrino

#endif // _IO_OUT_STREAM
