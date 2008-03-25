#ifndef _PUBLIC_CHANNEL
#define _PUBLIC_CHANNEL

namespace neutrino {


class IString;
class ITuple;
class IValue;
class IMethodDictionary;


enum ValueType {
  vtInteger, vtString, vtTuple, vtUnknown
};


template <class C> inline bool is(IValue obj);


template <class C> inline C cast(IValue obj);


class IValue {
public:
  inline ValueType type();
  inline IValue nothing();
protected:
  IMethodDictionary &methods() { return *methods_; }
  void *origin() { return origin_; }
private:
  inline IValue(IMethodDictionary &methods, void *origin);
  friend class MethodDictionaryImpl;
  IMethodDictionary *methods_;
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
class IMethodDictionary {
public:
  virtual ValueType type(IValue *that) = 0;
  virtual int value(IInteger *that) = 0;
  virtual int length(IString *that) = 0;
  virtual const char *c_str(IString *that) = 0;
  virtual int length(ITuple *that) = 0;
  virtual IValue get(ITuple *that, int index) = 0;
};


IValue::IValue(IMethodDictionary &methods, void *origin)
  : methods_(&methods)
  , origin_(origin) { }


ValueType IValue::type() {
  return methods().type(this);
}


IValue IValue::nothing() {
  return IValue(methods(), 0);
}


int IInteger::value() {
  return methods().value(this);
}


int IString::length() {
  return methods().length(this);
}


const char *IString::c_str() {
  return methods().c_str(this);
}


int ITuple::length() {
  return methods().length(this);
}


IValue ITuple::operator[](int index) {
  return methods().get(this, index);
}


template <class C> C cast(IValue obj) {
  if (!is<C>(obj)) obj = obj.nothing();
  return *reinterpret_cast<C*>(&obj);
}


template <> inline bool is<IString>(IValue obj) { return obj.type() == vtString; }
template <> inline bool is<ITuple>(IValue obj) { return obj.type() == vtTuple; }
template <> inline bool is<IInteger>(IValue obj) { return obj.type() == vtInteger; }


} // neutrino

#endif // _PUBLIC_CHANNEL
