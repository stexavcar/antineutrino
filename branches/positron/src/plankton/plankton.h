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
      typedef Type (*type_t)(Value that);
      typedef bool (*eq_t)(Value that, Value other);
      typedef void *(*impl_id_t)(Value that);
      type_t type;
      eq_t eq;
      impl_id_t impl_id;
    };
    struct IntegerDTable {
      typedef int32_t (*value_t)(Integer that);
      IntegerDTable(value_t _value) : value(_value) { }
      value_t value;
    };
    struct StringDTable {
      typedef word (*length_t)(String that);
      typedef uint32_t (*get_t)(String that, word offset);
      typedef word (*compare_t)(String that, String other);
      StringDTable(length_t _length, get_t _get, compare_t _compare)
        : length(_length), get(_get), compare(_compare) { }
      length_t length;
      get_t get;
      compare_t compare;
    };
    struct ArrayDTable {
      typedef word (*length_t)(Array that);
      typedef Value (*get_t)(Array that, word index);
      ArrayDTable(length_t _length, get_t _get)
        : length(_length), get(_get) { }
      length_t length;
      get_t get;
    };
    struct ObjectDTable {
      typedef Value (*send_t)(Object that, String species, Array args,
          MessageData *data, bool is_synchronous);
      ObjectDTable(send_t _send) : send(_send) { }
      send_t send;
    };
    struct SeedDTable {
      typedef bool (*seed_iterator_t)(String key, Value value, void *data);
      typedef String (*species_t)(Seed that);
      typedef Value (*get_attribute_t)(Seed that, String key);
      typedef void *(*grow_t)(Seed that, String species);
      typedef bool (*for_each_attribute_t)(Seed that, seed_iterator_t iter, void *data);
      SeedDTable(species_t _species, get_attribute_t _get_attribute,
          grow_t _grow, for_each_attribute_t _for_each_attribute)
        : species(_species), get_attribute(_get_attribute), grow(_grow)
        , for_each_attribute(_for_each_attribute) { }
      species_t species;
      get_attribute_t get_attribute;
      grow_t grow;
      for_each_attribute_t for_each_attribute;
    };
    ValueDTable value;
    IntegerDTable *integer;
    StringDTable *string;
    ArrayDTable *array;
    ObjectDTable *object;
    SeedDTable *seed;
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
  inline int32_t value() const { return dtable()->integer->value(*this); }
  inline Integer(word data, DTable *dtable) : Value(data, dtable) { }
  static const Value::Type kTypeTag = Value::vtInteger;
private:
  friend class Value;
  static Value::DTable *literal_adapter();
};


class Null : public Value {
public:
  inline Null(word data, DTable *dtable) : Value(data, dtable) { }
  static inline Null get() { return Null(0, null_dtable()); }
  static const Value::Type kTypeTag = Value::vtNull;
private:
  static Value::DTable *null_dtable();
};


class String : public Value {
public:
  inline word length() const { return dtable()->string->length(*this); }

  // Returns the i'th character of this string.  A '\0' will be returned
  // as the first character after the end of the string and access
  // beyond that may give any result, including program termination.
  inline uint32_t operator[](word index) const { return dtable()->string->get(*this, index); }

  inline bool operator==(String other) const { return dtable()->string->compare(*this, other) == 0; }
  inline bool operator!=(String other) const { return dtable()->string->compare(*this, other) != 0; }
  inline bool operator<(String other) const { return dtable()->string->compare(*this, other) < 0; }
  inline bool operator<=(String other) const { return dtable()->string->compare(*this, other) <= 0; }
  inline bool operator>(String other) const { return dtable()->string->compare(*this, other) > 0; }
  inline bool operator>=(String other) const { return dtable()->string->compare(*this, other) >= 0; }
  inline String(const char *str) : Value(str) { }
  inline String(word data, DTable *dtable) : Value(data, dtable) { }
  inline String() : Value() { }
  static const Value::Type kTypeTag = Value::vtString;
  static word generic_string_compare(String that, String other);
private:
  friend class Value;
  static Value::DTable *char_ptr_dtable();
};

class LiteralArray;

class Array : public Value {
public:
  inline word length() const { return dtable()->array->length(*this); }
  inline Value operator[](word index) const { return dtable()->array->get(*this, index); }
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
  inline Value send(String name, Array args = Array::empty(), MessageData *data = NULL) { return dtable()->object->send(*this, name, args, data, true); }
  inline void send_async(String name, Array args = Array::empty(), MessageData *data = NULL) { dtable()->object->send(*this, name, args, data, false); }
  inline Object(word data, DTable *table) : Value(data, table) { }
  inline Object() : Value() { }
  static const Value::Type kTypeTag = Value::vtObject;
};


class Seed : public Value {
public:
  inline String species() const { return dtable()->seed->species(*this); }
  inline Value operator[](String key) const { return dtable()->seed->get_attribute(*this, key); }
  typedef Value::DTable::SeedDTable::seed_iterator_t iterator_t;
  inline bool for_each_attribute(iterator_t iter, void *data) const { return dtable()->seed->for_each_attribute(*this, iter, data); }
  template <typename T>
  inline T *grow() const { return static_cast<T*>(dtable()->seed->grow(*this, T::species())); }
  inline Seed(word data, DTable *table) : Value(data, table) { }
  inline Seed() : Value() { }
  static const Value::Type kTypeTag = Value::vtSeed;
  static bool belongs_to(Seed seed, p::String species);
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
  static IClassRegistryEntry *lookup(p::String species);
  virtual void *new_instance(Seed seed) = 0;
  virtual p::String species() = 0;
  virtual p::String super_class() = 0;
protected:
  static void register_class(IClassRegistryEntry *entry);
private:
  static hash_map<p::String, IClassRegistryEntry*> &registry();
  friend class ClassRegistry;
};


template <typename T>
class ClassRegistryEntry : public IClassRegistryEntry {
public:
  inline ClassRegistryEntry() { register_class(this); }
  typedef T *(*constructor_t)(p::Seed seed);
  virtual p::String species() { return T::species(); }
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
