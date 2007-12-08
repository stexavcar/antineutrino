#include "compiler/ast-inl.h"
#include "heap/heap.h"
#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"

namespace neutrino {

#ifdef DEBUG

uint32_t Class::tag_of(Data *value) {
  if (is<Signal>(value)) {
    switch (cast<Signal>(value)->type()) {
#define MAKE_SIGNAL_TYPE_CASE(n, NAME, Name, info) case Signal::NAME: return NAME##_TYPE;
FOR_EACH_SIGNAL_TYPE(MAKE_SIGNAL_TYPE_CASE)
#undef MAKE_SIGNAL_TYPE_CASE
      default: UNREACHABLE(); return SIGNAL_TYPE;
    }
  } else {
    return cast<Value>(value)->gc_safe_type();
  }
}

const char *Class::tag_name(uint32_t tag) {
  switch (tag) {
#define MAKE_TYPE_CASE(n, NAME, Name, info) case NAME##_TYPE: return #NAME;
FOR_EACH_DECLARED_TYPE(MAKE_TYPE_CASE)
#undef MAKE_TYPE_CASE
    default: return "<illegal>";
  }
}

const char *Class::class_name(uint32_t tag) {
  switch (tag) {
#define MAKE_TYPE_CASE(n, NAME, Name, info) case NAME##_TYPE: return #Name;
FOR_EACH_DECLARED_TYPE(MAKE_TYPE_CASE)
#undef MAKE_TYPE_CASE
    default: return "<illegal>";
  }
}

#endif // DEBUG

MAKE_ENUM_INFO_HEADER(InstanceType)
#define MAKE_ENTRY(n, NAME, Name, info) MAKE_ENUM_INFO_ENTRY(NAME##_TYPE)
FOR_EACH_DECLARED_TYPE(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

MAKE_ENUM_INFO_HEADER(Signal::Type)
#define MAKE_ENTRY(n, NAME, Name, info) MAKE_ENUM_INFO_ENTRY(Signal::NAME)
FOR_EACH_SIGNAL_TYPE(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

// -----------------------
// --- P r i n t i n g ---
// -----------------------

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

static void write_smi_short_on(Smi *obj, string_buffer &buf) {
  buf.printf("%", obj->value());
}

static void write_string_short_on(String *obj, Data::WriteMode mode, string_buffer &buf) {
  if (mode != Data::UNQUOTED) buf.append('"');
  for (uint32_t i = 0; i < obj->length(); i++)
    buf.append(obj->at(i));
  if (mode != Data::UNQUOTED) buf.append('"');
}

static void write_class_short_on(Class *obj, string_buffer &buf) {
  buf.append("#<class ");
  obj->name()->write_on(buf, Data::UNQUOTED);
  buf.append(">");
}

static void write_instance_short_on(Instance *obj, string_buffer &buf) {
  Value *class_name = obj->chlass()->name();
  if (is<String>(class_name)) {
    buf.append("#<");
    if (cast<String>(class_name)->starts_with_vowel()) {
      buf.append("an ");
    } else {
      buf.append("a ");
    }
    class_name->write_on(buf, Data::UNQUOTED);
    buf.append(">");
  } else {
    buf.append("#<instance>");
  }
}

static void write_syntax_tree_on(SyntaxTree *obj, string_buffer &buf) {
  buf.append('<');
  obj->unparse_on(buf);
  buf.append('>');
}

static void write_object_short_on(Object *obj, Data::WriteMode mode, string_buffer &buf) {
  uint32_t instance_type = obj->chlass()->instance_type();
  switch (instance_type) {
  case STRING_TYPE:
    write_string_short_on(cast<String>(obj), mode, buf);
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
  case DICTIONARY_TYPE:
    buf.append("#<dictionary>");
    break;
  case CODE_TYPE:
    buf.append("#<code>");
    break;
  case CLASS_TYPE:
    write_class_short_on(cast<Class>(obj), buf);
    break;
  case METHOD_TYPE:
    buf.append("#<method>");
    break;
  case INSTANCE_TYPE:
    write_instance_short_on(cast<Instance>(obj), buf);
    break;
#define MAKE_CASE(n, NAME, Name, info) case NAME##_TYPE:
FOR_EACH_SYNTAX_TREE_TYPE(MAKE_CASE)
#undef MAKE_CASE
    write_syntax_tree_on(cast<SyntaxTree>(obj), buf);
    break;
  default:
    UNHANDLED(InstanceType, instance_type);
  }
}

static void write_signal_short_on(Signal *obj, string_buffer &buf) {
  switch (obj->type()) {
  case Signal::NOTHING:
    buf.append("@<nothing>");
    break;
  default:
    UNHANDLED(Signal::Type, obj->type());
  }
}

static void write_data_short_on(Data *obj, Data::WriteMode mode, string_buffer &buf) {
  if (is<Smi>(obj)) {
    write_smi_short_on(cast<Smi>(obj), buf);
  } else if (is<Object>(obj)) {
    write_object_short_on(cast<Object>(obj), mode, buf);
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
  obj->tree()->unparse_on(buf);
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

static void write_method_on(Method *obj, string_buffer &buf) {
  buf.append("method ");
  obj->name()->write_short_on(buf);
}

static void write_object_on(Object *obj, Data::WriteMode mode, string_buffer &buf) {
  switch (obj->type()) {
  case TUPLE_TYPE:
    write_tuple_on(cast<Tuple>(obj), buf);
    break;
  case LAMBDA_TYPE:
    write_lambda_on(cast<Lambda>(obj), buf);
    break;
  case DICTIONARY_TYPE:
    write_dictionary_on(cast<Dictionary>(obj), buf);
    break;
  case METHOD_TYPE:
    write_method_on(cast<Method>(obj), buf);
  default:
    write_object_short_on(obj, mode, buf);
    break;
  }
}

static void write_data_on(Data *obj, Data::WriteMode mode, string_buffer &buf) {
  if (is<Object>(obj)) {
    write_object_on(cast<Object>(obj), mode, buf);
  } else {
    write_data_short_on(obj, mode, buf);
  }
}

void Data::write_on(string_buffer &buf, Data::WriteMode mode) {
  write_data_on(this, mode, buf);
}

void Data::write_short_on(string_buffer &buf, Data::WriteMode mode) {
  write_data_short_on(this, mode, buf);
}


// -------------------------------
// --- D i s a s s e m b l e r ---
// -------------------------------

static void disassemble_buffer(uint16_t *data, uint32_t size,
    Tuple *literals, string_buffer &buf) {
  uint32_t pc = 0;
  while (pc < size) {
    buf.printf("%{ 3} ", pc);
    switch (data[pc]) {
      case OC_ARGUMENT:
        buf.printf("argument %", data[pc + 1]);
        pc += OpcodeInfo<OC_ARGUMENT>::kSize;
        break;
      case OC_LOCAL:
        buf.printf("local %", data[pc + 1]);
        pc += OpcodeInfo<OC_LOCAL>::kSize;
        break;
      case OC_GLOBAL: {
        scoped_string name(literals->at(data[pc + 1])->to_string());
        buf.printf("global %", name.chars());
        pc += OpcodeInfo<OC_GLOBAL>::kSize;
        break;
      }
      case OC_BUILTIN: {
        buf.printf("built-in %", data[pc + 1]);
        pc += OpcodeInfo<OC_BUILTIN>::kSize;
        break;
      }
      case OC_PUSH: {
        scoped_string value(literals->at(data[pc + 1])->to_string());
        buf.printf("push %", value.chars());
        pc += OpcodeInfo<OC_PUSH>::kSize;
        break;
      }
      case OC_SLAP:
        buf.printf("slap %", data[pc + 1]);
        pc += OpcodeInfo<OC_SLAP>::kSize;
        break;
      case OC_POP:
        buf.printf("pop %", data[pc + 1]);
        pc += OpcodeInfo<OC_POP>::kSize;
        break;
      case OC_CALL: {
        buf.printf("call %", data[pc + 1]);
        pc += OpcodeInfo<OC_CALL>::kSize;
        break;
      }
      case OC_INVOKE: {
        scoped_string name(literals->at(data[pc + 1])->to_string());
        buf.printf("invoke %", name.chars());
        pc += OpcodeInfo<OC_INVOKE>::kSize;
        break;
      }
      case OC_CLOSURE:
        buf.printf("closure % %", data[pc + 1], data[pc + 2]);
        pc += OpcodeInfo<OC_CLOSURE>::kSize;
        break;
      case OC_IF_TRUE:
        buf.printf("if_true %", data[pc + 1]);
        pc += OpcodeInfo<OC_IF_TRUE>::kSize;
        break;
      case OC_GOTO:
        buf.printf("goto %", data[pc + 1]);
        pc += OpcodeInfo<OC_GOTO>::kSize;
        break;
      case OC_VOID:
        buf.append("void");
        pc += OpcodeInfo<OC_VOID>::kSize;
        break;
      case OC_NULL:
        buf.append("null");
        pc += OpcodeInfo<OC_NULL>::kSize;
        break;
      case OC_RETURN:
        buf.append("return");
        pc += OpcodeInfo<OC_RETURN>::kSize;
        break;
      case OC_CONCAT:
        buf.printf("concat %", data[pc + 1]);
        pc += OpcodeInfo<OC_CONCAT>::kSize;
        break;
      case OC_CHKHGT:
        buf.printf("check height %", data[pc + 1]);
        pc += OpcodeInfo<OC_CHKHGT>::kSize;
        break;
      default:
        UNHANDLED(Opcode, data[pc]);
        return;
    }
    buf.append("\n");
  }
}

string Lambda::disassemble() {
  uint32_t size = cast<Code>(code())->length();
  uint16_t *data = &cast<Code>(code())->at(0);
  string_buffer buf;
  disassemble_buffer(data, size, cast<Tuple>(literals()), buf);
  return buf.to_string();
}


// ---------------
// --- S i z e ---
// ---------------

uint32_t Object::size_in_memory() {
  InstanceType instance_type = type();
  switch (instance_type) {
  case TRUE_TYPE: case FALSE_TYPE: case VOID_TYPE: case NULL_TYPE:
    return Singleton::kSize;
  case LAMBDA_TYPE:
    return Lambda::kSize;
  case BUILTIN_CALL_TYPE:
    return BuiltinCall::kSize;
  case CLASS_TYPE:
    return Class::kSize;
  case TUPLE_TYPE:
    return Tuple::size_for(cast<Tuple>(this)->length());
  case STRING_TYPE:
    return String::size_for(cast<String>(this)->length());
  case STACK_TYPE:
    return Stack::size_for(cast<Stack>(this)->height());
  case CODE_TYPE: case BUFFER_TYPE:
    return AbstractBuffer::size_for(cast<AbstractBuffer>(this)->size<uint8_t>());
#define MAKE_CASE(n, NAME, Name, name) case NAME##_TYPE: return Name::kSize;
FOR_EACH_GENERATABLE_TYPE(MAKE_CASE)
#undef MAKE_CASE
  default:
    UNHANDLED(InstanceType, instance_type);
    return 0;
  }
}


// ---------------------------
// --- V a l i d a t i o n ---
// ---------------------------

#ifdef DEBUG

// In order to validate, the following must hold about an object.  All
// fields that were set on construction must hold values of the type
// required by those fields.  All fields whose value may change during
// execution must hold valid non-signal pointers.

static void validate_tuple(Tuple *obj) {
  for (uint32_t i = 0; i < obj->length(); i++)
    GC_SAFE_CHECK_IS_C(VALIDATION, Value, obj->at(i));
}

#define VALIDATE_FIELD(Type, name, Name, Class)                      \
  GC_SAFE_CHECK_IS_C(VALIDATION, Type, cast<Class>(obj)->name());

static void validate_object(Object *obj) {
  GC_SAFE_CHECK_IS_C(VALIDATION, Class, obj->chlass());
  InstanceType type = obj->gc_safe_type();
  switch (type) {
    case TUPLE_TYPE:
      validate_tuple(cast<Tuple>(obj));
      break;
    case LAMBDA_TYPE:
      FOR_EACH_LAMBDA_FIELD(VALIDATE_FIELD, Lambda)
      if (!is<Smi>(cast<Lambda>(obj)->code())) {
        GC_SAFE_CHECK_IS_C(VALIDATION, Code, cast<Lambda>(obj)->code());
        GC_SAFE_CHECK_IS_C(VALIDATION, Tuple, cast<Lambda>(obj)->literals());
      }
      break;
    case CLASS_TYPE:
      FOR_EACH_CLASS_FIELD(VALIDATE_FIELD, Class)
      break;
    case QUOTE_TEMPLATE_TYPE:
      FOR_EACH_QUOTE_TEMPLATE_FIELD(VALIDATE_FIELD, QuoteTemplate)
      break;
    case BUILTIN_CALL_TYPE:
    case UNQUOTE_EXPRESSION_TYPE:
    case CODE_TYPE: case STRING_TYPE: case VOID_TYPE: case TRUE_TYPE:
    case FALSE_TYPE: case NULL_TYPE: case STACK_TYPE:
      break;
#define MAKE_CASE(n, NAME, Name, name)                               \
    case NAME##_TYPE:                                                \
      FOR_EACH_##NAME##_FIELD(VALIDATE_FIELD, Name);                 \
      break;
FOR_EACH_GENERATABLE_TYPE(MAKE_CASE)
#undef MAKE_CASE
    default:
      UNHANDLED(InstanceType, type);
  }
}

#undef VALIDATE_FIELD

void Value::validate() {
  if (is<Object>(this)) validate_object(cast<Object>(this));
}

#endif


// -------------------------
// --- I t e r a t i o n ---
// -------------------------

#define VISIT(field) visitor.visit_field(pointer_cast<Value*>(&field))
#define VISIT_FIELD(Type, name, Name, Class) visitor.visit_field(pointer_cast<Value*>(&cast<Class>(this)->name()));

void Object::for_each_field(FieldVisitor &visitor) {
  InstanceType type = this->type();
  visitor.visit_field(reinterpret_cast<Value**>(&header()));
  switch (type) {
    case STRING_TYPE: case CODE_TYPE: case TRUE_TYPE: case FALSE_TYPE:
    case VOID_TYPE: case NULL_TYPE:
      return;
    case TUPLE_TYPE:
      for (uint32_t i = 0; i < cast<Tuple>(this)->length(); i++)
        VISIT(cast<Tuple>(this)->at(i));
      break;
    case BUILTIN_CALL_TYPE:
      FOR_EACH_BUILTIN_CALL_FIELD(VISIT_FIELD, BuiltinCall);
      break;
    case LAMBDA_TYPE:
      FOR_EACH_LAMBDA_FIELD(VISIT_FIELD, Lambda)
      break;
    case CLASS_TYPE:
      FOR_EACH_CLASS_FIELD(VISIT_FIELD, Class)
      break;
    case STACK_TYPE:
      cast<Stack>(this)->for_each_stack_field(visitor);
      break;
#define MAKE_CASE(n, NAME, Name, name)                               \
    case NAME##_TYPE:                                                \
      FOR_EACH_##NAME##_FIELD(VISIT_FIELD, Name);                    \
      break;
FOR_EACH_GENERATABLE_TYPE(MAKE_CASE)
#undef MAKE_CASE
    default:
      UNHANDLED(InstanceType, type);
  }
}

#undef VISIT
#undef VISIT_FIELD

// -------------------
// --- L a m b d a ---
// -------------------

Value *Lambda::call(Task *task) {
  Interpreter &interpreter = Runtime::current().interpreter();
  return interpreter.call(this, task);
}

Data *Lambda::clone(Heap &heap) {
  return heap.new_lambda(argc(), code(), literals(), tree());
}


// -----------------------
// --- E q u a l i t y ---
// -----------------------

bool Value::is_key() {
  switch (type()) {
    case STRING_TYPE: case VOID_TYPE: case NULL_TYPE: case TRUE_TYPE:
    case FALSE_TYPE: case SMI_TYPE: case SYMBOL_TYPE:
      return true;
    default:
      return false;
  }
}

bool Value::is_identical(Value *that) {
  if (this == that) return true;
  if (this->type() != that->type()) return false;
  if (this->is_key()) return this->equals(that);
  else return false;
}

bool Value::equals(Value *that) {
  ASSERT(this->is_key());
  ASSERT(that->is_key());
  if (this == that) return true;
  if (this->type() != that->type()) return false;
  InstanceType type = this->type();
  switch (type) {
  case STRING_TYPE:
    return cast<String>(this)->string_equals(cast<String>(that));
  case SMI_TYPE: case SYMBOL_TYPE:
    return this == that;
  case VOID_TYPE:
    return is<Void>(that);
  case NULL_TYPE:
    return is<Null>(that);
  case TRUE_TYPE:
    return is<True>(that);
  case FALSE_TYPE:
    return is<False>(that);
  default:
    UNHANDLED(InstanceType, type);
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


// -------------------
// --- S t r i n g ---
// -------------------

// TODO(1): Implement proper unicode vowel/consonant predicate
bool String::starts_with_vowel() {
  if (length() == 0) return false;
  uint32_t chr = at(0);
  if ('A' <= chr && chr <= 'Z') chr += 'a' - 'A';
  switch (chr) {
  case 'a': case 'e': case 'i': case 'o': case 'u':
    return true;
  default:
    return false;
  }
}


// ---------------------------
// --- D i c t i o n a r y ---
// ---------------------------

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

uint32_t Dictionary::size() {
  return table()->length() / 2;
}


// -----------------
// --- C l a s s ---
// -----------------

bool Class::is_empty() {
  return super() == Smi::from_int(0);
}


} // namespace neutrino
