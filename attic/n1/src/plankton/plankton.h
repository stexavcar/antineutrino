#ifndef _PLANKTON_PLANKTON
#define _PLANKTON_PLANKTON

#include "utils/global.h"
#include "utils/string.h"

namespace neutrino {
namespace plankton {

class Value {
public:
  enum Type { vtInteger, vtString, vtNull, vtArray, vtObject, vtSeed };
  struct Handler {
    virtual Value::Type value_type(Value that) = 0;
    virtual bool value_eq(Value that, Value other) = 0;
    virtual void *value_impl_id(Value that) = 0;
  };
  bool match(const Pattern &pattern) const;
  inline Type type() const;
  inline void *impl_id() const;
  inline bool operator==(Value that) const;
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
  struct Handler {
    virtual int32_t integer_value(Integer that) = 0;
  };
  inline int32_t value() const;
  inline Integer(word data, DTable *dtable) : Value(data, dtable) { }
  static const Value::Type kTypeTag = Value::vtInteger;
private:
  friend class Value;
  static DTable *literal_adapter();
};


class Null : public Value {
public:
  inline Null(word data, DTable *dtable) : Value(data, dtable) { }
  static inline Null get() { return Null(0, null_dtable()); }
  static const Value::Type kTypeTag = Value::vtNull;
private:
  static DTable *null_dtable();
};


class String : public Value {
public:

  struct Handler {
    virtual word string_length(String that) = 0;
    virtual uint32_t string_get(String that, word offset) = 0;
    virtual word string_compare(String that, String other) = 0;
  };

  inline word length() const;

  // Returns the i'th character of this string.  A '\0' will be returned
  // as the first character after the end of the string and access
  // beyond that may give any result, including program termination.
  inline uint32_t operator[](word index) const;

  inline bool operator==(String other) const;
  inline bool operator!=(String other) const;
  inline bool operator<(String other) const;
  inline bool operator<=(String other) const;
  inline bool operator>(String other) const;
  inline bool operator>=(String other) const;
  inline String(const char *str);
  inline String(word data, DTable *dtable) : Value(data, dtable) { }
  inline String() : Value() { }
  static const Value::Type kTypeTag = Value::vtString;
  static word generic_string_compare(String that, String other);
private:
  friend class Value;
  static DTable *char_ptr_dtable();
};

class LiteralArray;

class Array : public Value {
public:

  struct Handler {
    virtual word array_length(Array that) = 0;
    virtual Value array_get(Array that, word index) = 0;
  };

  inline word length() const;
  inline Value operator[](word index) const;
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
  static DTable *empty_array_adapter();
};


class LiteralArray : public Array {
public:
  inline LiteralArray(Value v0, Value v1, Value v2, Value v3, Value v4, Value v5);
  ~LiteralArray() { }
  static const word kMaxSize = 6;
private:
  static DTable *literal_array_adapter();
  Value values_[kMaxSize];
};


class IMessageResource {
public:
  virtual ~IMessageResource() { }
};


class Object : public Value {
public:

  struct Handler {
    virtual Value object_send(Object that, String species, Array args,
        MessageData *data, bool is_synchronous) = 0;
  };

  inline Value send(String name, Array args = Array::empty(), MessageData *data = NULL);
  inline void send_async(String name, Array args = Array::empty(), MessageData *data = NULL);
  inline Object(word data, DTable *table) : Value(data, table) { }
  inline Object() : Value() { }
  static const Value::Type kTypeTag = Value::vtObject;
};


class Seed : public Value {
public:
  typedef bool (*attribute_callback_t)(String key, Value value, void *data);
  struct Handler {
    virtual String seed_species(Seed that) = 0;
    virtual Value seed_get_attribute(Seed that, String key) = 0;
    virtual void *seed_grow(Seed that, String species) = 0;
    virtual bool seed_for_each_attribute(Seed that, attribute_callback_t iter,
        void *data) = 0;
    virtual Value seed_send(Seed that, String name, Value arg) = 0;
  };
  inline String species() const;
  inline Value operator[](String key) const;
  inline bool for_each_attribute(attribute_callback_t iter, void *data) const;
  template <typename T> inline T *grow() const;
  inline Value send(String name, Value arg) const;
  inline Seed(word data, DTable *table) : Value(data, table) { }
  inline Seed() : Value() { }
  static const Value::Type kTypeTag = Value::vtSeed;
  static bool belongs_to(Seed seed, p::String species);
};

class DTable {
public:
  DTable();
  Value::Handler *value;
  Integer::Handler *integer;
  String::Handler *string;
  Array::Handler *array;
  Object::Handler *object;
  Seed::Handler *seed;
};

Value::Type Value::type() const {
  return dtable()->value->value_type(*this);
}

void *Value::impl_id() const {
  return dtable()->value->value_impl_id(*this);
}

bool Value::operator==(Value that) const {
  return dtable()->value->value_eq(*this, that);
}

int32_t Integer::value() const {
  return dtable()->integer->integer_value(*this);
}

word String::length() const {
  return dtable()->string->string_length(*this);
}

uint32_t String::operator[](word index) const {
  return dtable()->string->string_get(*this, index);
}

bool String::operator==(String other) const {
  return dtable()->string->string_compare(*this, other) == 0;
}

bool String::operator!=(String other) const {
  return dtable()->string->string_compare(*this, other) != 0;
}

bool String::operator<(String other) const {
  return dtable()->string->string_compare(*this, other) < 0;
}

bool String::operator<=(String other) const {
  return dtable()->string->string_compare(*this, other) <= 0;
}

bool String::operator>(String other) const {
  return dtable()->string->string_compare(*this, other) > 0;
}

bool String::operator>=(String other) const {
  return dtable()->string->string_compare(*this, other) >= 0;
}

word Array::length() const {
  return dtable()->array->array_length(*this);
}

Value Array::operator[](word index) const {
  return dtable()->array->array_get(*this, index);
}

Value Object::send(String name, Array args, MessageData *data) {
  return dtable()->object->object_send(*this, name, args, data, true);
}

void Object::send_async(String name, Array args, MessageData *data) {
  dtable()->object->object_send(*this, name, args, data, false);
}

String Seed::species() const {
  return dtable()->seed->seed_species(*this);
}

Value Seed::operator[](String key) const {
  return dtable()->seed->seed_get_attribute(*this, key);
}

bool Seed::for_each_attribute(attribute_callback_t iter, void *data) const {
  return dtable()->seed->seed_for_each_attribute(*this, iter, data);
}

template <typename T>
T *Seed::grow() const {
  return static_cast<T*>(dtable()->seed->seed_grow(*this, T::species()));
}

Value Seed::send(String name, Value arg) const {
  return dtable()->seed->seed_send(*this, name, arg);
}

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
