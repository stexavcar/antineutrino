#ifndef _PUBLIC_CHANNEL
#define _PUBLIC_CHANNEL

namespace neutrino {


class NInteger;
class NString;
class NTuple;
class NValue;


enum ValueType {
  vtInteger, vtString, vtTuple, vtUnknown
};


/**
 * This class holds all nontrivial methods used in the api.  This is
 * done in order to factor our all nontrivial functionality, while
 * still being able to pass objects around by value.  As a user of
 * this api you don't need to know that this class exists.
 */
struct ValueDTable {
  ValueType (NValue::*value_type_)();
  int (NInteger::*integer_value_)();
  int (NString::*string_length_)();
  char (NString::*string_get_)(int index);
  const char *(NString::*string_c_str_)();
  int (NTuple::*tuple_length_)();
  NValue (NTuple::*tuple_get_)(int index);
};


/**
 * Type query operator.  Is<X>(o) return true if o is an X, false
 * otherwise.
 */
template <class C> inline bool is(NValue obj);


/**
 * Type conversion operator.  Cast<X>(o) returns o viewed as an X if
 * o is an X, otherwise the empty value.
 */
template <class C> inline C cast(NValue obj);


class NValue {
public:
  inline ValueType type() { return (this->*(methods().value_type_))(); }
  inline NValue nothing();
protected:
  ValueDTable &methods() { return *methods_; }
private:
  inline NValue(ValueDTable &methods, void *origin);
  void *origin() { return origin_; }
  
  friend class ApiUtils;
  ValueDTable *methods_;
  void *origin_;
};


NValue NValue::nothing() {
  return NValue(methods(), 0);
}


class NInteger : public NValue {
public:
  inline int value() { return (this->*(methods().integer_value_))(); }
};


class NString : public NValue {
public:
  inline int length() { return (this->*(methods().string_length_))(); }
  inline const char *c_str() { return (this->*(methods().string_c_str_))(); }
  inline char operator[](int index) { return ((this->*(methods().string_get_)))(index); }
};


class NTuple : public NValue {
public:
  inline int length() { return ((this->*(methods().tuple_length_)))(); }
  inline NValue operator[](int index) { return ((this->*(methods().tuple_get_)))(index); }
};


class IExternalChannel {
public:
  virtual int receive(NValue message) = 0;
};


class IExternalChannelConfiguration {
public:
  virtual void bind(IExternalChannel &channel) = 0;
};


// -------------------------------------
// --- I m p l e m e n t a t i o n s ---
// -------------------------------------


NValue::NValue(ValueDTable &methods, void *origin)
  : methods_(&methods)
  , origin_(origin) { }


template <class C> C cast(NValue obj) {
  if (!is<C>(obj)) obj = obj.nothing();
  return *reinterpret_cast<C*>(&obj);
}


template <> inline bool is<NString>(NValue obj) {
  return obj.type() == vtString;
}


template <> inline bool is<NTuple>(NValue obj) {
  return obj.type() == vtTuple;
}


template <> inline bool is<NInteger>(NValue obj) {
  return obj.type() == vtInteger;
}


} // neutrino

#endif // _PUBLIC_CHANNEL
