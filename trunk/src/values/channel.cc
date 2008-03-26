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
    case tString:
      return vtString;
    case tSmi:
      return vtInteger;
    case tTuple:
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

char MethodDictionaryImpl::get(IString *that, int index) {
  return open<FString>(that)->at(index);
}

const char *MethodDictionaryImpl::c_str(IString *that) {
  FString *str = open<FString>(that);
  uword length = str->length();
  char *result = new char[length + 1];
  for (uword i = 0; i < length; i++)
    result[i] = str->at(i);
  result[length] = '\0';
  return result;
}

int MethodDictionaryImpl::length(ITuple *that) {
  return open<FTuple>(that)->length();
}

IValue MethodDictionaryImpl::get(ITuple *that, int index) {
  FImmediate *result = open<FTuple>(that)->at(index);
  return new_value(result);
}

} // neutrino
