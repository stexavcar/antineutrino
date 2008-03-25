#include "io/image-inl.h"
#include "io/out-stream.h"
#include "values/values-inl.h"

namespace neutrino {


FrozenHeap::FrozenHeap(ImageOutputStream &stream) 
  : data_(stream.data()) { }


ImageValue *FrozenHeap::cook(RawFrozenValue *obj) {
  ImageValue *raw_value = reinterpret_cast<ImageValue*>(obj);
  if (is<ImageSmi>(raw_value)) return raw_value;
  UNREACHABLE();
  return 0;
}


static inline RawFrozenValue *freeze(void *obj) {
  return reinterpret_cast<RawFrozenValue*>(obj);
}


RawFrozenValue *ImageOutputStream::marshal(Immediate *obj) {
  if (is<Smi>(obj)) {
    return freeze(obj);
  } else {
    UNREACHABLE();
  }
  return 0;
}


} // neutrino
