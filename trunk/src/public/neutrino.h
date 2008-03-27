#ifndef _PUBLIC_CHANNEL
#define _PUBLIC_CHANNEL

namespace neutrino {


class IString;
class ITuple;
class IValue;
class MethodDictionary;


enum ValueType {
  vtInteger, vtString, vtTuple, vtUnknown
};


template <class C> inline bool is(IValue obj);


template <class C> inline C cast(IValue obj);


class IValue {
public:
  inline ValueType type();
  inline IValue nothing();

  MethodDictionary &methods() { return *methods_; }
  void *origin() { return origin_; }

private:
  inline IValue(MethodDictionary &methods, void *origin);
  friend class MethodDictionaryImpl;
  MethodDictionary *methods_;
  void *origin_;
};


class IInteger : public IValue {
public:
  inline int value();
};


class IString : public IValue {
public:
  inline int length();
  inline const char *c_str();
  inline char operator[](int index);
};


class ITuple : public IValue {
public:
  inline int length();
  inline IValue operator[](int index);
};


class IMessage {
public:

};


/**
 * This class holds all nontrivial methods used in the api.  This is
 * done in order to factor our all nontrivial functionality, while
 * still being able to pass objects around by value.  As a user of
 * this api you don't need to know that this class exists.
 */
struct MethodDictionary {
  ValueType (IValue::*value_type_)();
  int (IInteger::*integer_value_)();
  int (IString::*string_length_)();
  char (IString::*string_get_)(int index);
  const char *(IString::*string_c_str_)();
  int (ITuple::*tuple_length_)();
  IValue (ITuple::*tuple_get_)(int index);
};


IValue::IValue(MethodDictionary &methods, void *origin)
  : methods_(&methods)
  , origin_(origin) { }


ValueType IValue::type() {
  return (this->*(methods().value_type_))();
}


IValue IValue::nothing() {
  return IValue(methods(), 0);
}


int IInteger::value() {
  return (this->*(methods().integer_value_))();
}


int IString::length() {
  return (this->*(methods().string_length_))();
}


const char *IString::c_str() {
  return (this->*(methods().string_c_str_))();
}


char IString::operator[](int index) {
  return ((this->*(methods().string_get_)))(index);
}


int ITuple::length() {
  return ((this->*(methods().tuple_length_)))();
}


IValue ITuple::operator[](int index) {
  return ((this->*(methods().tuple_get_)))(index);
}


template <class C> C cast(IValue obj) {
  if (!is<C>(obj)) obj = obj.nothing();
  return *reinterpret_cast<C*>(&obj);
}


template <> inline bool is<IString>(IValue obj) {
  return obj.type() == vtString;
}


template <> inline bool is<ITuple>(IValue obj) {
  return obj.type() == vtTuple;
}


template <> inline bool is<IInteger>(IValue obj) {
  return obj.type() == vtInteger;
}


} // neutrino

#endif // _PUBLIC_CHANNEL
