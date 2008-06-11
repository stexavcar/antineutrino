#ifndef _PUBLIC_PLANKTON
#define _PUBLIC_PLANKTON

namespace plankton {


class Integer;
template <typename T> class Proxy;
class String;
class Tuple;

class Value {
public:
  enum Tag { vtInteger, vtString, vtTuple, vtProxy, vtNull, vtUnknown };
  
  /**
   * This class holds all nontrivial methods used in the api.  This is
   * done in order to factor our all nontrivial functionality, while
   * still being able to pass objects around by value.  As a user of
   * this api you don't need to know that this class exists.
   */
  struct DTable {
    Tag (Value::*value_type_)();
    int (Integer::*integer_value_)();
    unsigned (String::*string_length_)();
    unsigned (String::*string_get_)(unsigned index);
    const char *(String::*string_c_str_)();
    unsigned (Tuple::*tuple_length_)();
    Value (Tuple::*tuple_get_)(unsigned index);
    void *(Value::*proxy_deref_)(unsigned size);
  };
  
  inline void *origin() { return origin_; }
  DTable &dtable() { return *dtable_; }

  inline Tag type() { return (this->*(dtable().value_type_))(); }
  inline Value nothing() { return Value(dtable(), 0); }
  
  inline Value(DTable &dtable, void *origin) : dtable_(&dtable), origin_(origin) { };

private:
  DTable *dtable_;
  void *origin_;
};


/**
 * Type query operator.  Is<X>(o) return true if o is an X, false
 * otherwise.
 */
template <class C> inline bool is(Value obj) {
  return obj.type() == C::kTag;
}


template <class C> C cast(Value obj) {
  if (!is<C>(obj)) obj = obj.nothing();
  return *reinterpret_cast<C*>(&obj);
}


class Integer : public Value {
public:
  static const Value::Tag kTag = vtInteger;
  inline int value() { return (this->*(dtable().integer_value_))(); }
};


class String : public Value {
public:
  static const Value::Tag kTag = vtString;
  inline unsigned length() { return (this->*(dtable().string_length_))(); }
  inline unsigned operator[](int index) { return (this->*(dtable().string_get_))(index); }
  inline const char *c_str() { return (this->*(dtable().string_c_str_))(); }
};


class Tuple : public Value {
public:
  static const Value::Tag kTag = vtTuple;
  inline unsigned length() { return (this->*(dtable().tuple_length_))(); }
  inline Value operator[](int index) { return (this->*(dtable().tuple_get_))(index); }  
};


template <typename T>
class Proxy : public Value {
public:
  static const Value::Tag kTag = vtProxy;
  inline const T &get() { return *static_cast<T*>(((this->*(dtable().proxy_deref_)))(sizeof(T))); }
  inline void set(const T &value) { *static_cast<T*>(((this->*(dtable().proxy_deref_)))(sizeof(T))) = value; }
};


class Null : public Value {
  static const Value::Tag kTag = vtNull;
};


class IBuilder {
public:
  virtual Integer new_integer(int value) = 0;
  virtual String new_string(const char *data, unsigned length) = 0;
  virtual Null get_null() = 0;
  template <typename T> Proxy<T> new_proxy();
private:
  virtual Value new_raw_proxy(unsigned size) = 0;
};


template <typename T> Proxy<T> IBuilder::new_proxy() {
  return cast< Proxy<T> >(new_raw_proxy(sizeof(T)));
}


} // namespace plankton

#endif // _PUBLIC_PLANKTON
