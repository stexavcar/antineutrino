#include "io/image-inl.h"
#include "io/in-stream-inl.h"
#include "io/out-stream-inl.h"
#include "values/values-inl.h"

namespace neutrino {


FrozenHeap::FrozenHeap(Serializer &stream) 
  : data_(stream.data()) {
  ASSERT(stream.has_been_flushed());
}


FImmediate *FrozenHeap::cook(RawFValue *obj) {
  FImmediate *raw_value = reinterpret_cast<FImmediate*>(obj);
  if (is<FSmi>(raw_value)) {
    return raw_value;
  } else {
    ASSERT(is<FObject>(raw_value));
    word *start = data().data();
    uword offset = ValuePointer::offset_of(obj);
    uword val = ValuePointer::tag_as_object(start + offset);
    return reinterpret_cast<FImmediate*>(val);
  }
  return 0;
}


static inline RawFValue *freeze(void *obj) {
  return reinterpret_cast<RawFValue*>(obj);
}


Serializer::Serializer()
  : has_been_flushed_(false) { }


RawFValue *Serializer::marshal_object_shallow(Object *obj) {
  RawFValue *result = freeze(ValuePointer::tag_offset_as_object(buffer().length()));
  InstanceType type = obj->type();
  switch (type) {
    case tString: {
      String *that = cast<String>(obj);
      uword size = FString::size_for(that->length());
      FString *img = buffer().allocate<FString>(tString, size);
      img->set_length(that->length());
      for (uword i = 0; i < that->length(); i++)
        img->set(i, that->at(i));
      break;
    }
    case tTuple: {
      Tuple *that = cast<Tuple>(obj);
      uword length = that->length();
      uword size = FTuple::size_for(length);
      FTuple *img = buffer().allocate<FTuple>(tTuple, size);
      img->set_length(length);
      for (uword i = 0; i < length; i++)
        img->set_raw(i, that->get(i));
      break;
    }
    default:
      UNHANDLED(InstanceType, type);
  }
  return result;
}


void Serializer::marshal_object_deep(FObject *obj) {
  InstanceType type = obj->type();
  switch (type) {
    case tString:
      break;
    case tTuple: {
      FTuple *that = image_raw_cast<FTuple>(obj);
      uword length = that->length();
      for (uword i = 0; i < length; i++) {
        Value *elm = that->get_raw(i);
        USE(elm);
      }
      break;
    }
    default:
      UNHANDLED(InstanceType, type);
  }
}


RawFValue *Serializer::marshal(Immediate *obj) {
  if (is<Smi>(obj)) {
    return freeze(obj);
  } else if (is<Object>(obj)) {
    return marshal_object_shallow(cast<Object>(obj));
  } else {
    UNREACHABLE();
  }
  return 0;
}


void Serializer::flush() {
  has_been_flushed_ = true;
  ImageIterator<ExtensibleHeap, ExtensibleHeap::Data> iter(buffer());
  while (iter.has_next()) {
    FObject *next = iter.next();
    marshal_object_deep(next);
  }
}


} // neutrino
