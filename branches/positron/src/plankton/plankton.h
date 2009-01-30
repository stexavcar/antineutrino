#ifndef _PLANKTON_PLANKTON
#define _PLANKTON_PLANKTON

#include "utils/global.h"
#include "utils/string.h"

namespace neutrino {
namespace plankton {

class Value {
public:
  enum Type { vtInteger, vtString, vtNull, vtArray, vtObject, vtSeed };

  class DTable {
  public:
    DTable();
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
      Value (*send)(Object that, String name, Array args,
          MessageData *data, bool is_synchronous);
    };
    struct SeedDTable {
      String (*oid)(Seed that);
      Value (*get)(Seed that, String key);
      bool (*set)(Seed that, String key, Value value);
      void *(*grow)(Seed that, String oid);
    };
    ValueDTable value;
    IntegerDTable integer;
    StringDTable string;
    ArrayDTable array;
    ObjectDTable object;
    SeedDTable seed;
  };

  bool match(Pattern &pattern);
  inline Type type() const { return dtable()->value.type(*this); }
  inline void *impl_id() const { return dtable()->value.impl_id(*this); }
  inline bool operator==(Value that) const { return dtable()->value.eq(*this, that); }
  word data() const { return data_; }
  DTable *dtable() const { return dtable_; }
  bool is_empty() { return dtable_ == NULL; }

  inline Value(word data, DTable *dtable) : data_(data), dtable_(dtable) { }
  inline Value() : data_(0), dtable_(NULL) { }
  inline Value(word value);
  inline Value(const char *str);

private:
  word data_;
  DTable *dtable_;
};

class Integer : public Value {
public:
  inline int32_t value() const { return dtable()->integer.value(*this); }
  inline Integer(word data, DTable *dtable) : Value(data, dtable) { }
  static const Value::Type kTypeTag = Value::vtInteger;
private:
  friend class Value;
  static Value::DTable *literal_adapter();
};


class Null : public Value {
public:
  inline Null(word data, DTable *dtable) : Value(data, dtable) { }
  static const Value::Type kTypeTag = Value::vtNull;
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
  inline String(const char *str) : Value(str) { }
  inline String(word data, DTable *dtable) : Value(data, dtable) { }
  inline String() : Value() { }
  static const Value::Type kTypeTag = Value::vtString;
  static word generic_string_compare(String that, String other);
private:
  friend class Value;
  static Value::DTable *char_ptr_adapter();
};

class LiteralArray;

class Array : public Value {
public:
  inline word length() const { return dtable()->array.length(*this); }
  inline Value operator[](word index) const { return dtable()->array.get(*this, index); }
  inline bool set(word index, Value value) { return dtable()->array.set(*this, index, value); }
  inline Array(word data, DTable *dtable) : Value(data, dtable) { }
  inline Array() : Value() { }
  static Array empty() { return Array(0, empty_array_adapter()); }
  static const Value::Type kTypeTag = Value::vtArray;

  // Returns a transient array that can be used as an argument to a
  // call, but disappears immediately after the invokation it is used
  // in completes.
  static inline LiteralArray of(Value v0 = Value(), Value v1 = Value(),
      Value v2 = Value(), Value v3 = Value(), Value v4 = Value(),
      Value v5 = Value());

private:
  static Value::DTable *empty_array_adapter();
};


class LiteralArray : public Array {
public:
  inline LiteralArray(Value v0, Value v1, Value v2, Value v4, Value v4, Value v5);
  ~LiteralArray() { }
  static const word kMaxSize = 6;
private:
  static Value::DTable *literal_array_adapter();
  Value values_[kMaxSize];
};


class IMessageResource {
public:
  virtual ~IMessageResource() { }
};


class Object : public Value {
public:
  inline Value send(String name, Array args = Array::empty(), MessageData *data = NULL) { return dtable()->object.send(*this, name, args, data, true); }
  inline void send_async(String name, Array args = Array::empty(), MessageData *data = NULL) { dtable()->object.send(*this, name, args, data, false); }
  inline Object(word data, DTable *table) : Value(data, table) { }
  inline Object() : Value() { }
  static const Value::Type kTypeTag = Value::vtObject;
};


class Seed : public Value {
public:
  inline String oid() const { return dtable()->seed.oid(*this); }
  inline Value operator[](String key) const { return dtable()->seed.get(*this, key); }
  inline bool set(String key, Value value) { return dtable()->seed.set(*this, key, value); }
  template <typename T>
  inline T *grow() const { return static_cast<T*>(dtable()->seed.grow(*this, T::oid())); }
  inline Seed(word data, DTable *table) : Value(data, table) { }
  inline Seed() : Value() { }
  static const Value::Type kTypeTag = Value::vtSeed;
  static bool belongs_to(Seed seed, p::String oid);
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


class IClass {
public:
  static p::String super_class() { return p::String(); }
};


class IClassRegistryEntry {
public:
  static IClassRegistryEntry *lookup(p::String oid);
  virtual void *new_instance(Seed seed) = 0;
  virtual p::String name() = 0;
  virtual p::String super_class() = 0;
protected:
  static void register_class(IClassRegistryEntry *entry);
private:
  friend class ClassRegistry;
};


template <typename T>
class ClassRegistryEntry : public IClassRegistryEntry {
public:
  inline ClassRegistryEntry() { register_class(this); }
  typedef T *(*constructor_t)(p::Seed seed);
  virtual p::String name() { return T::oid(); }
  virtual p::String super_class() { return T::super_class(); }
  virtual void *new_instance(Seed seed) { return T::construct(seed); }
private:
};


#define REGISTER_SERVICE(name, allocator)                            \
  p::ServiceRegistryEntry SEMI_STATIC_JOIN(__entry__, __LINE__)(#name, allocator)


#define REGISTER_CLASS(Type)                                         \
  p::ClassRegistryEntry<Type> SEMI_STATIC_JOIN(Type##__entry__, __LINE__)

} // namespace plankton
} // namespace neutrino


#endif // _PLANKTON_PLANKTON
