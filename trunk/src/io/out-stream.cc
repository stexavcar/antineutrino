#include "io/image-inl.h"
#include "io/out-stream.h"
#include "values/values-inl.h"

namespace neutrino {


FrozenHeap::FrozenHeap(ImageOutputStream &stream) 
  : data_(stream.data()) { }


FImmediate *FrozenHeap::cook(RawFValue *obj) {
  FImmediate *raw_value = reinterpret_cast<FImmediate*>(obj);
  if (is<FSmi>(raw_value)) return raw_value;
  UNREACHABLE();
  return 0;
}


static inline RawFValue *freeze(void *obj) {
  return reinterpret_cast<RawFValue*>(obj);
}


RawFValue *ImageOutputStream::marshal(Immediate *obj) {
  if (is<Smi>(obj)) {
    return freeze(obj);
  } else {
    UNREACHABLE();
  }
  return 0;
}


} // neutrino
