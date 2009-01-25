#ifndef _PLANKTON_PLANKTON
#define _PLANKTON_PLANKTON

#include "utils/global.h"
#include "utils/string.h"

namespace neutrino {
namespace plankton {

class Value {
public:
  enum Type { vtInteger, vtString, vtNull, vtVoid, vtArray, vtObject };

  class DTable {
  public:
    struct ValueDTable {
      Type (*type)(Value that);
      bool (*eq)(Value that, Value other);
      void *(*impl_id)(Value that);
    };
    struct IntegerDTable {
      int32_t (*value)(Integer that);
    };
    struct StringDTable {
      word (*length)(String that);
      uint32_t (*get)(String that, word offset);
      word (*compare)(String that, String other);
    };
    struct ArrayDTable {
      word (*length)(Array that);
      Value (*get)(Array that, word index);
      bool (*set)(Array that, word index, Value value);
    };
    struct ObjectDTable {
      Value (*send)(Object tthat, String name, Array args,
          bool is_synchronous);
    };
    ValueDTable value;
    IntegerDTable integer;
    StringDTable string;
    ArrayDTable array;
    ObjectDTable object;
  };

  inline Type type() const { return dtable()->value.type(*this); }
  inline void *impl_id() const { return dtable()->value.impl_id(*this); }
  inline bool operator==(Value that) const { return dtable()->value.eq(*this, that); }
  word data() const { return data_; }
  DTable *dtable() const { return dtable_; }
  bool is_empty() { return dtable_ == NULL; }

  inline Value(word data, DTable *dtable) : data_(data), dtable_(dtable) { }
  inline Value() : data_(0), dtable_(NULL) { }

private:
  word data_;
  DTable *dtable_;
};

class Integer : public Value {
public:
  inline int32_t value() const { return dtable()->integer.value(*this); }
  inline Integer(word data, DTable *dtable) : Value(data, dtable) { }
  static const Value::Type kTypeTag = Value::vtInteger;
};


class Null : public Value {
public:
  inline Null(word data, DTable *dtable) : Value(data, dtable) { }
  static const Value::Type kTypeTag = Value::vtNull;
};


class Void : public Value {
public:
  inline Void(word data, DTable *dtable) : Value(data, dtable) { }
  static const Value::Type kTypeTag = Value::vtVoid;
};


class String : public Value {
public:
  inline word length() const { return dtable()->string.length(*this); }

  // Returns the i'th character of this string.  A '\0' will be returned
  // as the first character after the end of the string and access
  // beyond that may give any result, including program termination.
  inline uint32_t operator[](word index) const { return dtable()->string.get(*this, index); }

  inline bool operator==(String other) const { return dtable()->string.compare(*this, other) == 0; }
  inline bool operator!=(String other) const { return dtable()->string.compare(*this, other) != 0; }
  inline bool operator<(String other) const { return dtable()->string.compare(*this, other) < 0; }
  inline bool operator<=(String other) const { return dtable()->string.compare(*this, other) <= 0; }
  inline bool operator>(String other) const { return dtable()->string.compare(*this, other) > 0; }
  inline bool operator>=(String other) const { return dtable()->string.compare(*this, other) >= 0; }
  inline String(const char *str) : Value(reinterpret_cast<word>(str), c_string_dtable()) { }
  inline String(word data, DTable *dtable) : Value(data, dtable) { }
  inline String() : Value() { }
  static const Value::Type kTypeTag = Value::vtString;
private:
  static Value::DTable *c_string_dtable();
  static Value::DTable *c_string_dtable_;
};


class Array : public Value {
public:
  inline word length() const { return dtable()->array.length(*this); }
  inline Value operator[](word index) const { return dtable()->array.get(*this, index); }
  inline bool set(word index, Value value) { return dtable()->array.set(*this, index, value); }
  inline Array(word data, DTable *dtable) : Value(data, dtable) { }
  inline Array() : Value() { }
  static const Value::Type kTypeTag = Value::vtArray;
};


class Object : public Value {
public:
  inline Value send_sync(String name, Array args = Array()) { return dtable()->object.send(*this, name, args, true); }
  inline void send_async(String name, Array args = Array()) { dtable()->object.send(*this, name, args, false); }
  inline Object(word data, DTable *table) : Value(data, table) { }
  inline Object() : Value() { }
  static const Value::Type kTypeTag = Value::vtObject;
};


class ServiceRegistry {
public:
  static Object lookup(String name);
};


class ServiceRegistryEntry {
public:
  typedef Object (*instance_allocator)();
  inline ServiceRegistryEntry(const char *name, instance_allocator alloc);
  Object get_instance();
private:
  friend class ServiceRegistry;
  static ServiceRegistryEntry *first() { return first_; }
  const char *name() { return name_; }
  ServiceRegistryEntry *prev() { return prev_; }
  const char *name_;
  instance_allocator alloc_;
  ServiceRegistryEntry *prev_;
  static ServiceRegistryEntry *first_;
  bool has_instance_;
  Object instance_;
};


#define REGISTER_SERVICE(name, allocator)                            \
  static p::ServiceRegistryEntry SEMI_STATIC_JOIN(__entry__, __LINE__)(#name, allocator)


} // namespace plankton
} // namespace neutrino


#endif // _PLANKTON_PLANKTON
