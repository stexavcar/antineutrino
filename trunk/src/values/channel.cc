#include "values/channel.h"
#include "io/image-inl.h"

namespace neutrino {

template <class C> C *MethodDictionaryImpl::open(IValue *obj) {
  return static_cast<C*>(obj->origin());
}

void *MethodDictionaryImpl::close(ImageValue *obj) {
  return static_cast<void*>(obj);
}

IValue MethodDictionaryImpl::new_value(void *origin) {
  return IValue(*this, origin);
}

ValueType MethodDictionaryImpl::type(IValue *that) {
  InstanceType type = open<ImageValue>(that)->type();
  switch (type) {
    case STRING_TYPE:
      return vtString;
    case SMI_TYPE:
      return vtInteger;
    case TUPLE_TYPE:
      return vtTuple;
    default:
      return vtUnknown;
  }
}

int MethodDictionaryImpl::value(IInteger *that) {
  return open<ImageSmi>(that)->value();
}

int MethodDictionaryImpl::length(IString *that) {
  return open<ImageString>(that)->length();
}

const char *MethodDictionaryImpl::c_str(IString *that) {
  return NULL;
}

int MethodDictionaryImpl::length(ITuple *that) {
  return open<ImageTuple>(that)->length();
}

IValue MethodDictionaryImpl::get(ITuple *that, int index) {
  ImageValue *result = open<ImageTuple>(that)->at(index);
  return new_value(result);
}

} // neutrino
