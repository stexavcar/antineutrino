#include "heap/heap.h"
#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "io/ast-inl.h"
#include "runtime/runtime-inl.h"

namespace neutrino {

#ifdef DEBUG

uint32_t Class::tag_of(Data *value) {
  if (is<Smi>(value)) {
    return SMI_TYPE;
  } else if (is<Signal>(value)) {
    switch (cast<Signal>(value)->type()) {
#define MAKE_SIGNAL_TYPE_CASE(NAME, Name) case Signal::NAME: return NAME##_TYPE;
FOR_EACH_SIGNAL_TYPE(MAKE_SIGNAL_TYPE_CASE)
#undef MAKE_SIGNAL_TYPE_CASE
      default: UNREACHABLE(); return SIGNAL_TYPE;
    }
  } else {
    return cast<Object>(value)->chlass()->instance_type();
  }
}

const char *Class::tag_name(uint32_t tag) {
  switch (tag) {
#define MAKE_TYPE_CASE(NAME, Name) case NAME##_TYPE: return #NAME;
FOR_EACH_DECLARED_TYPE(MAKE_TYPE_CASE)
#undef MAKE_TYPE_CASE
    default: return "<illegal>";
  }
}

const char *Class::class_name(uint32_t tag) {
  switch (tag) {
#define MAKE_TYPE_CASE(NAME, Name) case NAME##_TYPE: return #Name;
FOR_EACH_DECLARED_TYPE(MAKE_TYPE_CASE)
#undef MAKE_TYPE_CASE
    default: return "<illegal>";
  }
}

bool Value::is_key() {
  if (is<Smi>(this)) {
    return true;
  } if (is<Object>(this)) {
    switch (cast<Object>(this)->chlass()->instance_type()) {
    case STRING_TYPE: case VOID_TYPE: case NULL_TYPE: case TRUE_TYPE:
    case FALSE_TYPE:
      return true;
    }
  }
  return false;
}

#endif // DEBUG

MAKE_ENUM_INFO_HEADER(InstanceType)
#define MAKE_ENTRY(NAME, Name) MAKE_ENUM_INFO_ENTRY(NAME##_TYPE)
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
  uint32_t instance_type = obj->chlass()->instance_type();
  switch (instance_type) {
  case STRING_TYPE:
    write_string_short_on(cast<String>(obj), buf);
    break;
  case TUPLE_TYPE:
    buf.append("#<tuple>");
    break;
  case LAMBDA_TYPE:
    buf.append("#<lambda>");
    break;
  case VOID_TYPE:
    buf.append("void");
    break;
  case NULL_TYPE:
    buf.append("null");
    break;
  case TRUE_TYPE:
    buf.append("true");
    break;
  case FALSE_TYPE:
    buf.append("false");
    break;
  case LITERAL_TYPE:
    buf.append("#<literal>");
    break;
  case DICTIONARY_TYPE:
    buf.append("#<dictionary>");
    break;
  case CODE_TYPE:
    buf.append("#<code>");
    break;
  default:
    UNHANDLED(InstanceType, instance_type);
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
  switch (obj->chlass()->instance_type()) {
  case TUPLE_TYPE:
    write_tuple_on(cast<Tuple>(obj), buf);
    break;
  case LAMBDA_TYPE:
    write_lambda_on(cast<Lambda>(obj), buf);
    break;
  case LITERAL_TYPE:
    write_literal_on(cast<Literal>(obj), buf);
    break;
  case DICTIONARY_TYPE:
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
  uint32_t instance_type = cast<Object>(this)->chlass()->instance_type();
  switch (instance_type) {
  case STRING_TYPE:
    if (!is<String>(that)) return false;
    return cast<String>(this)->string_equals(cast<String>(that));
  case VOID_TYPE:
    return is<Void>(that);
  case NULL_TYPE:
    return is<Null>(that);
  case TRUE_TYPE:
    return is<True>(that);
  case FALSE_TYPE:
    return is<False>(that);
  default:
    UNHANDLED(InstanceType, instance_type);
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
