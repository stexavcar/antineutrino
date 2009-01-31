#include "plankton/plankton-inl.h"
#include "plankton/match.h"
#include "utils/string-inl.h"
#include "utils/hash-map-inl.h"

namespace neutrino {
namespace plankton {

// --- D u m m y   D T a b l e ---

Value::DTable::DTable() {
  value.type = NULL;
  value.eq = NULL;
  value.impl_id = NULL;
  integer.value = NULL;
  string.length = NULL;
  string.get = NULL;
  array.length = NULL;
  array.get = NULL;
  array.set = NULL;
  object.send = NULL;
}

bool Value::match(Pattern &pattern) {
  switch (type()) {
    case vtArray:
      return pattern.match_array(cast<Array>(*this));
    case vtInteger:
      return pattern.match_integer(cast<Integer>(*this));
    case vtString:
      return pattern.match_string(cast<String>(*this));
    case vtNull:
      return pattern.match_null(cast<Null>(*this));
    case vtObject:
      return pattern.match_object(cast<Object>(*this));
    case vtSeed:
      assert false;
      return false;
  };
  return false;
}

// --- I n t e g e r   L i t e r a l s ---

class IntegerLiteralAdapter : public Value::DTable {
public:
  IntegerLiteralAdapter();
private:
  static int32_t integer_value(Integer that);
  static Value::Type value_type(Value that);
};

IntegerLiteralAdapter::IntegerLiteralAdapter() {
  value.type = value_type;
  integer.value = integer_value;
}

int32_t IntegerLiteralAdapter::integer_value(Integer that) {
  return that.data();
}

Value::Type IntegerLiteralAdapter::value_type(Value that) {
  return Value::vtInteger;
}

Value::DTable *Integer::literal_adapter() {
  static IntegerLiteralAdapter instance;
  return &instance;
}

// --- S t r i n g   L i t e r a l s ---

class CharPtrAdapter : public Value::DTable {
public:
  CharPtrAdapter();
private:
  static Value::Type value_type(Value that);
  static word string_length(String str);
  static uint32_t string_get(String str, word index);
  static word string_compare(String that, String other);
  static const char *open(String str);
};

CharPtrAdapter::CharPtrAdapter() {
  value.type = value_type;
  string.length = string_length;
  string.compare = string_compare;
  string.get = string_get;
}

Value::DTable *String::char_ptr_adapter() {
  static Value::DTable *value = NULL;
  if (value == NULL)
    value = new CharPtrAdapter();
  return value;
}

Value::Type CharPtrAdapter::value_type(Value that) {
  return Value::vtString;
}

const char *CharPtrAdapter::open(String str) {
  return reinterpret_cast<const char*>(str.data());
}

word CharPtrAdapter::string_length(String str) {
  return strlen(open(str));
}

uint32_t CharPtrAdapter::string_get(String str, word index) {
  return open(str)[index];
}

word CharPtrAdapter::string_compare(String that, String other) {
  return String::generic_string_compare(that, other);
}

// --- E m p t y   A r r a y s ---

class EmptyArrayAdapter : public Value::DTable {
public:
  EmptyArrayAdapter();
  static Value::Type value_type(Value that);
private:
  static word array_length(Array that);
};

EmptyArrayAdapter::EmptyArrayAdapter() {
  array.length = array_length;
  value.type = value_type;
}

word EmptyArrayAdapter::array_length(Array that) {
  return 0;
}

Value::Type EmptyArrayAdapter::value_type(Value that) {
  return Value::vtArray;
}

Value::DTable *Array::empty_array_adapter() {
  static Value::DTable *value = NULL;
  if (value == NULL)
    value = new EmptyArrayAdapter();
  return value;
}

// --- L i t e r a l   A r r a y s ---

class LiteralArrayAdapter : public Value::DTable {
public:
  LiteralArrayAdapter();
private:
  static word array_length(Array that);
  static Value array_get(Array that, word index);
};

LiteralArrayAdapter::LiteralArrayAdapter() {
  value.type = EmptyArrayAdapter::value_type;
  array.length = array_length;
  array.get = array_get;
}

word LiteralArrayAdapter::array_length(Array that) {
  Value *elms = reinterpret_cast<Value*>(that.data());
  for (word i = 0; i < LiteralArray::kMaxSize; i++)
    if (elms[i].is_empty()) return i;
  return LiteralArray::kMaxSize;
}

Value LiteralArrayAdapter::array_get(Array that, word index) {
  Value *elms = reinterpret_cast<Value*>(that.data());
  return elms[index];
}

Value::DTable *LiteralArray::literal_array_adapter() {
  static Value::DTable *value = NULL;
  if (value == NULL)
    value = new LiteralArrayAdapter();
  return value;
}

ServiceRegistryEntry *ServiceRegistryEntry::first_ = NULL;

word String::generic_string_compare(String that, String other) {
  for (word i = 0; true; i++) {
    uint32_t ca = that[i];
    uint32_t cb = other[i];
    if (ca == cb) {
      if (ca == '\0')
        return 0;
    } else if (ca == '\0') {
      return -1;
    } else if (cb == '\0') {
      return 1;
    } else {
      return static_cast<word>(ca) - static_cast<word>(cb);
    }
  }
}

bool Seed::belongs_to(Seed that, String oid) {
  String current = that.oid();
  while (!current.is_empty()) {
    if (current == oid)
      return true;
    IClassRegistryEntry *entry = IClassRegistryEntry::lookup(current);
    if (entry == NULL)
      return false;
    current = entry->super_class();
  }
  return false;
}

Object ServiceRegistry::lookup(String name) {
  ServiceRegistryEntry *current = ServiceRegistryEntry::first();
  while (current != NULL) {
    if (name == current->name())
      return current->get_instance();
    current = current->prev();
  }
  return Object();
}

Object ServiceRegistryEntry::get_instance() {
  if (!has_instance_) {
    instance_ = alloc_();
    has_instance_ = true;
  }
  return instance_;
}

static hash_map<p::String, IClassRegistryEntry*> classes;
void IClassRegistryEntry::register_class(IClassRegistryEntry *entry) {
  classes.put(entry->name(), entry);
}

IClassRegistryEntry *IClassRegistryEntry::lookup(String oid) {
  return classes.get(oid, static_cast<IClassRegistryEntry*>(NULL));
}

MessageData::MessageData() { }
MessageData::~MessageData() { }

void MessageData::acquire_resource(IMessageResource &resource) {
  resources().append(&resource);
}

} // namespace plankton
} // namespace neutrino


namespace neutrino {

variant_type_impl<p::Value> variant_type_impl<p::Value>::kInstance;

void variant_type_impl<p::Value>::print_on(const void *data, string modifiers,
    string_stream &stream) {
  p::Value value = *static_cast<const p::Value*>(data);
  switch (value.type()) {
    case p::Value::vtNull:
      stream.add("null");
      break;
    case p::Value::vtArray: {
      p::Array array = cast<p::Array>(value);
      word length = array.length();
      stream.add("(");
      for (word i = 0; i < length; i++) {
        if (i > 0) stream.add(" ");
        stream.add("%", args(array[i]));
      }
      stream.add(")");
      break;
    }
    case p::Value::vtInteger: {
      word value = cast<p::Integer>(value).value();
      stream.add("%", args(value));
      break;
    }
    case p::Value::vtString: {
      stream.add('"');
      p::String str = cast<p::String>(value);
      for (word i = 0; str[i]; i++)
        stream.add(str[i]);
      stream.add('"');
      break;
    }
    case p::Value::vtSeed: {
      assert false;
      break;
    }
    case p::Value::vtObject:
      stream.add("#<an Object>");
      break;
  }
}

} // namespace neutrino
