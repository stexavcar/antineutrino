#include "plankton/plankton-inl.h"
#include "plankton/match.h"
#include "utils/string-inl.h"
#include "utils/hash-map-inl.h"

namespace neutrino {
namespace plankton {

// --- D u m m y   D T a b l e ---

DTable::DTable()
  : value(NULL)
  , integer(NULL)
  , string(NULL)
  , array(NULL)
  , object(NULL)
  , seed(NULL){
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

class IntegerLiteralAdapter : public DTable, public Integer::Handler {
public:
  IntegerLiteralAdapter();
  virtual int32_t integer_value(Integer that);
private:
  Value::Methods value_dtable_;
  static Value::Type value_type(Value that);
};

IntegerLiteralAdapter::IntegerLiteralAdapter()
  : value_dtable_(value_type, NULL, NULL) {
  integer = this;
  value = &value_dtable_;
}

int32_t IntegerLiteralAdapter::integer_value(Integer that) {
  return that.data();
}

Value::Type IntegerLiteralAdapter::value_type(Value that) {
  return Value::vtInteger;
}

DTable *Integer::literal_adapter() {
  static IntegerLiteralAdapter instance;
  return &instance;
}

// --- S t r i n g   L i t e r a l s ---

class CharPtrDTable : public DTable, public String::Handler {
public:
  CharPtrDTable();
  virtual word string_length(String str);
  virtual uint32_t string_get(String str, word index);
  virtual word string_compare(String that, String other);
private:
  static Value::Type value_type(Value that);
  static const char *open(String str);
  Value::Methods value_dtable_;
};

CharPtrDTable::CharPtrDTable()
  : value_dtable_(value_type, NULL, NULL) {
  string = this;
  value = &value_dtable_;
}

DTable *String::char_ptr_dtable() {
  static CharPtrDTable instance;
  return &instance;
}

Value::Type CharPtrDTable::value_type(Value that) {
  return Value::vtString;
}

const char *CharPtrDTable::open(String str) {
  return reinterpret_cast<const char*>(str.data());
}

word CharPtrDTable::string_length(String str) {
  return strlen(open(str));
}

uint32_t CharPtrDTable::string_get(String str, word index) {
  return open(str)[index];
}

word CharPtrDTable::string_compare(String that, String other) {
  return String::generic_string_compare(that, other);
}

// --- E m p t y   A r r a y s ---

class EmptyArrayAdapter : public DTable, public Array::Handler {
public:
  EmptyArrayAdapter();
  virtual word array_length(Array that);
  virtual Value array_get(Array that, word index);
  static Value::Type value_type(Value that);
private:
  Value::Methods value_dtable_;
};

EmptyArrayAdapter::EmptyArrayAdapter()
  : value_dtable_(value_type, NULL, NULL) {
  array = this;
  value = &value_dtable_;
}

word EmptyArrayAdapter::array_length(Array that) {
  return 0;
}

Value EmptyArrayAdapter::array_get(Array that, word index) {
  return Value();
}

Value::Type EmptyArrayAdapter::value_type(Value that) {
  return Value::vtArray;
}

DTable *Array::empty_array_adapter() {
  static EmptyArrayAdapter instance;
  return &instance;
}

// --- L i t e r a l   A r r a y s ---

class LiteralArrayAdapter : public DTable, public Array::Handler {
public:
  LiteralArrayAdapter();
  virtual word array_length(Array that);
  virtual Value array_get(Array that, word index);
private:
  Value::Methods value_dtable_;
};

LiteralArrayAdapter::LiteralArrayAdapter()
  : value_dtable_(EmptyArrayAdapter::value_type, NULL, NULL) {
  array = this;
  value = &value_dtable_;
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

DTable *LiteralArray::literal_array_adapter() {
  static LiteralArrayAdapter instance;
  return &instance;
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

bool Seed::belongs_to(Seed that, String species) {
  String current = that.species();
  while (!current.is_empty()) {
    if (current == species)
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

hash_map<p::String, IClassRegistryEntry*> &IClassRegistryEntry::registry() {
  static hash_map<p::String, IClassRegistryEntry*> instance;
  return instance;
}


void IClassRegistryEntry::register_class(IClassRegistryEntry *entry) {
  registry().put(entry->species(), entry);
}

IClassRegistryEntry *IClassRegistryEntry::lookup(String species) {
  return registry().get(species, static_cast<IClassRegistryEntry*>(NULL));
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

void variant_type_impl<p::Value>::print_on(const variant &that, string modifiers,
    string_stream &stream) {
  p::Value value(reinterpret_cast<word>(that.data_.u_pair.first),
      static_cast<p::DTable*>(that.data_.u_pair.second));
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
      word v = cast<p::Integer>(value).value();
      stream.add("%", args(v));
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
