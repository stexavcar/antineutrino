#ifndef _PUBLIC_CHANNEL
#define _PUBLIC_CHANNEL


namespace neutrino {


class IMessage;
class IMessageContext;
class IValueFactory;
template <typename T> class NProxy;
class NInteger;
class NString;
class NTuple;
class NValue;


enum ValueType {
  vtInteger, vtString, vtTuple, vtProxy, vtUnknown
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
  char (NString::*string_get_)(unsigned index);
  const char *(NString::*string_c_str_)();
  int (NTuple::*tuple_length_)();
  NValue (NTuple::*tuple_get_)(unsigned index);
  void *(NValue::*proxy_deref_)(unsigned size);
};


class NValue {
public:
  inline ValueType type() { return (this->*(methods().value_type_))(); }
  inline NValue nothing();
protected:
  ValueDTable &methods() { return *methods_; }
private:
  inline NValue(ValueDTable *methods, void *origin);
  void *origin() { return origin_; }
  
  friend class ApiUtils;
  ValueDTable *methods_;
  void *origin_;
};


/**
 * Type query operator.  Is<X>(o) return true if o is an X, false
 * otherwise.
 */
template <class C> inline bool is(NValue obj) {
  return obj.type() == C::kTag;
}


template <class C> C cast(NValue obj) {
  if (!is<C>(obj)) obj = obj.nothing();
  return *reinterpret_cast<C*>(&obj);
}


NValue NValue::nothing() {
  return NValue(methods_, 0);
}


class NInteger : public NValue {
public:
  static const ValueType kTag = vtInteger;
  inline int value() { return (this->*(methods().integer_value_))(); }
};


class NString : public NValue {
public:
  static const ValueType kTag = vtString;
  inline int length() { return (this->*(methods().string_length_))(); }
  inline const char *c_str() { return (this->*(methods().string_c_str_))(); }
  inline char operator[](unsigned index) { return ((this->*(methods().string_get_)))(index); }
};


class NTuple : public NValue {
public:
  static const ValueType kTag = vtTuple;
  inline int length() { return ((this->*(methods().tuple_length_)))(); }
  inline NValue operator[](unsigned index) { return ((this->*(methods().tuple_get_)))(index); }
};


template <typename T>
class NProxy : public NValue {
public:
  static const ValueType kTag = vtProxy;
  inline const T &get() { return *static_cast<T*>(((this->*(methods().proxy_deref_)))(sizeof(T))); }
  inline void set(const T &value) { *static_cast<T*>(((this->*(methods().proxy_deref_)))(sizeof(T))) = value; }
};


class NNull : public NValue {
public:
};


class IExternalChannel {
public:
  virtual NValue receive(IMessage &message) = 0;
};


class IExternalChannelConfiguration {
public:
  virtual void bind(IExternalChannel &channel) = 0;
};


class IMessage {
public:
  virtual NValue contents() = 0;
  virtual IMessageContext &context() = 0;
};


class IMessageContext {
public:
  virtual IValueFactory &factory() = 0;
};


class IValueFactory {
public:
  virtual NInteger new_integer(int value) = 0;
  virtual NNull get_null() = 0;
  virtual NString new_string(const char *data, unsigned length) = 0;
  template <typename T> NProxy<T> new_proxy();
private:
  virtual NValue new_raw_proxy(unsigned size) = 0;
};


// -------------------------------------
// --- I m p l e m e n t a t i o n s ---
// -------------------------------------


NValue::NValue(ValueDTable *methods, void *origin)
  : methods_(methods)
  , origin_(origin) { }


template <typename T> NProxy<T> IValueFactory::new_proxy() {
  return cast< NProxy<T> >(new_raw_proxy(sizeof(T)));
}


} // neutrino

#endif // _PUBLIC_CHANNEL
