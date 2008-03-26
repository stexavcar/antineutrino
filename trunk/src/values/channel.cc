#include "values/channel.h"
#include "io/image-inl.h"

namespace neutrino {

template <class C> C *MethodDictionaryImpl::open(IValue *obj) {
  return static_cast<C*>(obj->origin());
}

void *MethodDictionaryImpl::close(FImmediate *obj) {
  return static_cast<void*>(obj);
}

IValue MethodDictionaryImpl::new_value(void *origin) {
  return IValue(*this, origin);
}

ValueType MethodDictionaryImpl::type(IValue *that) {
  InstanceType type = open<FImmediate>(that)->type();
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
  return open<FSmi>(that)->value();
}

int MethodDictionaryImpl::length(IString *that) {
  return open<FString>(that)->length();
}

const char *MethodDictionaryImpl::c_str(IString *that) {
  return NULL;
}

int MethodDictionaryImpl::length(ITuple *that) {
  return open<FTuple>(that)->length();
}

IValue MethodDictionaryImpl::get(ITuple *that, int index) {
  FImmediate *result = open<FTuple>(that)->at(index);
  return new_value(result);
}

} // neutrino
