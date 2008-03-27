#include "values/channel.h"
#include "io/image-inl.h"

namespace neutrino {

static MethodDictionaryImpl kInstance;

class IValueImpl : public IValue {
public:
  ValueType type();
};

class IIntegerImpl : public IInteger {
public:
  int value();
};

class IStringImpl : public IString {
public:
  int length();
  char get(int index);
  const char *c_str();
};

class ITupleImpl : public ITuple {
public:
  int length();
  IValue get(int index);
};

MethodDictionaryImpl::MethodDictionaryImpl() {
  value_type_ = static_cast<ValueType (IValue::*)()>(&IValueImpl::type);
  integer_value_ = static_cast<int (IInteger::*)()>(&IIntegerImpl::value);
  string_length_ = static_cast<int (IString::*)()>(&IStringImpl::length);
  string_get_ = static_cast<char (IString::*)(int)>(&IStringImpl::get);
  string_c_str_ = static_cast<const char *(IString::*)()>(&IStringImpl::c_str);
  tuple_length_ = static_cast<int (ITuple::*)()>(&ITupleImpl::length);
  tuple_get_ = static_cast<IValue (ITuple::*)(int)>(&ITupleImpl::get);
}

template <class C> C *open(IValue *obj) {
  return static_cast<C*>(obj->origin());
}

void *MethodDictionaryImpl::close(FImmediate *obj) {
  return static_cast<void*>(obj);
}

IValue MethodDictionaryImpl::new_value(void *origin) {
  return IValue(*this, origin);
}

ValueType IValueImpl::type() {
  InstanceType type = open<FImmediate>(this)->type();
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

int IIntegerImpl::value() {
  return open<FSmi>(this)->value();
}

int IStringImpl::length() {
  return open<FString>(this)->length();
}

char IStringImpl::get(int index) {
  return open<FString>(this)->at(index);
}

const char *IStringImpl::c_str() {
  FString *str = open<FString>(this);
  uword length = str->length();
  char *result = new char[length + 1];
  for (uword i = 0; i < length; i++)
    result[i] = str->at(i);
  result[length] = '\0';
  return result;
}

int ITupleImpl::length() {
  return open<FTuple>(this)->length();
}

IValue ITupleImpl::get(int index) {
  FImmediate *result = open<FTuple>(this)->at(index);
  return kInstance.new_value(result);
}

} // neutrino
