#include "heap/heap.h"
#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "io/ast-inl.h"
#include "runtime/runtime-inl.h"

namespace neutrino {

#ifdef DEBUG

uint32_t Type::tag_of(Data *value) {
  if (is<Smi>(value)) {
    return SMI_TAG;
  } else if (is<Signal>(value)) {
    switch (cast<Signal>(value)->type()) {
#define MAKE_SIGNAL_TYPE_CASE(NAME, Name) case Signal::NAME: return NAME##_TAG;
FOR_EACH_SIGNAL_TYPE(MAKE_SIGNAL_TYPE_CASE)
#undef MAKE_SIGNAL_TYPE_CASE
      default: UNREACHABLE(); return SIGNAL_TAG;
    }
  } else {
    return cast<Object>(value)->type()->instance_type();
  }
}

const char *Type::tag_name(uint32_t tag) {
  switch (tag) {
#define MAKE_TYPE_CASE(NAME, Name) case NAME##_TAG: return #NAME;
FOR_EACH_DECLARED_TYPE(MAKE_TYPE_CASE)
#undef MAKE_TYPE_CASE
    default: return "<illegal>";
  }
}

const char *Type::class_name(uint32_t tag) {
  switch (tag) {
#define MAKE_TYPE_CASE(NAME, Name) case NAME##_TAG: return #Name;
FOR_EACH_DECLARED_TYPE(MAKE_TYPE_CASE)
#undef MAKE_TYPE_CASE
    default: return "<illegal>";
  }
}

bool Value::is_key() {
  if (is<Smi>(this)) {
    return true;
  } if (is<Object>(this)) {
    switch (cast<Object>(this)->type()->instance_type()) {
    case STRING_TAG: case VOID_TAG: case NULL_TAG: case TRUE_TAG:
    case FALSE_TAG:
      return true;
    }
  }
  return false;
}

#endif // DEBUG

MAKE_ENUM_INFO_HEADER(TypeTag)
#define MAKE_ENTRY(NAME, Name) MAKE_ENUM_INFO_ENTRY(NAME##_TAG)
FOR_EACH_DECLARED_TYPE(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

MAKE_ENUM_INFO_HEADER(Signal::Type)
#define MAKE_ENTRY(NAME, Name) MAKE_ENUM_INFO_ENTRY(Signal::NAME)
FOR_EACH_SIGNAL_TYPE(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

// --- P r i n t i n g ---

string Data::to_string() {
  string_buffer buf;
  write_on(buf);
  return buf.to_string();
}

string Data::to_short_string() {
  string_buffer buf;
  write_short_on(buf);
  return buf.to_string();
}

void Data::print(FILE *out) {
  string_buffer buf;
  write_on(buf);
  buf.append('\0');
  fprintf(out, "%s\n", buf.raw_string().chars());
}

static void write_smi_short_on(Smi *obj, string_buffer &buf) {
  buf.printf("%", obj->value());
}

static void write_string_short_on(String *obj, string_buffer &buf) {
  buf.append('"');
  for (uint32_t i = 0; i < obj->length(); i++)
    buf.append(obj->at(i));
  buf.append('"');
}

static void write_object_short_on(Object *obj, string_buffer &buf) {
  uint32_t instance_type = obj->type()->instance_type();
  switch (instance_type) {
  case STRING_TAG:
    write_string_short_on(cast<String>(obj), buf);
    break;
  case TUPLE_TAG:
    buf.append("#<tuple>");
    break;
  case LAMBDA_TAG:
    buf.append("#<lambda>");
    break;
  case VOID_TAG:
    buf.append("void");
    break;
  case NULL_TAG:
    buf.append("null");
    break;
  case TRUE_TAG:
    buf.append("true");
    break;
  case FALSE_TAG:
    buf.append("false");
    break;
  case LITERAL_TAG:
    buf.append("#<literal>");
    break;
  case DICTIONARY_TAG:
    buf.append("#<dictionary>");
    break;
  case CODE_TAG:
    buf.append("#<code>");
    break;
  default:
    UNHANDLED(TypeTag, instance_type);
  }
}

static void write_signal_short_on(Signal *obj, string_buffer &buf) {
  switch (obj->type()) {
  case Signal::NOTHING:
    buf.append("%<nothing>");
    break;
  default:
    UNHANDLED(Signal::Type, obj->type());
  }
}

static void write_data_short_on(Data *obj, string_buffer &buf) {
  if (is<Smi>(obj)) {
    write_smi_short_on(cast<Smi>(obj), buf);
  } else if (is<Object>(obj)) {
    write_object_short_on(cast<Object>(obj), buf);
  } else if (is<Signal>(obj)) {
    write_signal_short_on(cast<Signal>(obj), buf);
  } else {
    UNREACHABLE();
  }
}

static void write_tuple_on(Tuple *obj, string_buffer &buf) {
  buf.append('[');
  for (uint32_t i = 0; i < obj->length(); i++) {
    if (i > 0) buf.append(", ");
    obj->at(i)->write_short_on(buf);
  }
  buf.append(']');
}

static void write_lambda_on(Lambda *obj, string_buffer &buf) {
  buf.printf("fn #% -> ", static_cast<int32_t>(obj->argc()));
  obj->code()->write_on(buf);
}

static void write_literal_on(Literal *obj, string_buffer &buf) {
  obj->value()->write_on(buf);
}

static void write_dictionary_on(Dictionary *obj, string_buffer &buf) {
  buf.append('{');
  Dictionary::Iterator iter(obj);
  Dictionary::Iterator::Entry entry;
  bool first = true;
  while (iter.next(&entry)) {
    if (first) first = false;
    else buf.append(", ");
    entry.key->write_on(buf);
    buf.append(": ");
    entry.value->write_on(buf);
  }
  buf.append('}');
}

static void write_object_on(Object *obj, string_buffer &buf) {
  switch (obj->type()->instance_type()) {
  case TUPLE_TAG:
    write_tuple_on(cast<Tuple>(obj), buf);
    break;
  case LAMBDA_TAG:
    write_lambda_on(cast<Lambda>(obj), buf);
    break;
  case LITERAL_TAG:
    write_literal_on(cast<Literal>(obj), buf);
    break;
  case DICTIONARY_TAG:
    write_dictionary_on(cast<Dictionary>(obj), buf);
    break;
  default:
    write_object_short_on(obj, buf);
    break;
  }
}

static void write_data_on(Data *obj, string_buffer &buf) {
  if (is<Object>(obj)) {
    write_object_on(cast<Object>(obj), buf);
  } else {
    write_data_short_on(obj, buf);
  }
}

void Data::write_on(string_buffer &buf) {
  write_data_on(this, buf);
}

void Data::write_short_on(string_buffer &buf) {
  write_data_short_on(this, buf);
}

// --- L a m b d a ---

ref<Value> ref_traits<Lambda>::call() {
  Interpreter &interpreter = Runtime::current().interpreter();
  return interpreter.call(open(this));
}

// --- D i c t i o n a r y ---

bool Value::equals(Value *that) {
  ASSERT(this->is_key());
  ASSERT(that->is_key());
  if (is<Smi>(this)) return this == that;
  uint32_t instance_type = cast<Object>(this)->type()->instance_type();
  switch (instance_type) {
  case STRING_TAG:
    if (!is<String>(that)) return false;
    return cast<String>(this)->string_equals(cast<String>(that));
  case VOID_TAG:
    return is<Void>(that);
  case NULL_TAG:
    return is<Null>(that);
  case TRUE_TAG:
    return is<True>(that);
  case FALSE_TAG:
    return is<False>(that);
  default:
    UNHANDLED(TypeTag, instance_type);
    return false;
  }
}

bool String::string_equals(String *that) {
  if (this->length() != that->length())
    return false;
  for (uint32_t i = 0; i < this->length(); i++) {
    if (this->at(i) != that->at(i))
      return false;
  }
  return true;
}

struct DictionaryLookup {
  Value **value;
};

static bool lookup_key(Tuple *table, Value *key,
    DictionaryLookup &result) {
  ASSERT(key->is_key());
  for (uint32_t i = 0; i < table->length(); i += 2) {
    if (key->equals(table->at(i))) {
      result.value = &table->at(i + 1);
      return true;
    }
  }
  return false;
}

Data *Dictionary::get(Value *key) {
  DictionaryLookup lookup;
  if (lookup_key(this->table(), key, lookup)) return *lookup.value;
  else return Nothing::make();  
}

bool Dictionary::set(Value *key, Value *value) {
  DictionaryLookup lookup;
  if (lookup_key(this->table(), key, lookup)) {
    *lookup.value = value;
  } else {
    // Extend table with the new pair
    Tuple *table = this->table();
    uint32_t length = table->length();
    Data *new_table_val = Runtime::current().heap().new_tuple(length + 2);
    if (is<AllocationFailed>(new_table_val)) return false;
    Tuple *new_table = cast<Tuple>(new_table_val);
    for (uint32_t i = 0; i < length; i++)
      new_table->at(i) = table->at(i);
    new_table->at(length) = key;
    new_table->at(length + 1) = value;
    this->set_table(new_table);
  }
  return true;
}

} // namespace neutrino
