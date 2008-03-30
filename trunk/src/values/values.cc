#include "compiler/ast-inl.h"
#include "heap/heap.h"
#include "heap/ref-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"
#include "values/values-inl.h"

namespace neutrino {

#ifdef DEBUG

uword Layout::tag_of(Data *value) {
  if (is<Signal>(value)) {
    switch (cast<Signal>(value)->type()) {
#define MAKE_SIGNAL_TYPE_CASE(n, NAME, Name, info) case Signal::s##Name: return t##Name;
FOR_EACH_SIGNAL_TYPE(MAKE_SIGNAL_TYPE_CASE)
#undef MAKE_SIGNAL_TYPE_CASE
      default: UNREACHABLE(); return tSignal;
    }
  } else {
    return cast<Value>(value)->gc_safe_type();
  }
}

const char *Layout::tag_name(uword tag) {
  switch (tag) {
#define MAKE_TYPE_CASE(n, NAME, Name, info) case t##Name: return #NAME;
FOR_EACH_DECLARED_TYPE(MAKE_TYPE_CASE)
#undef MAKE_TYPE_CASE
    default: return "<illegal>";
  }
}

#endif // DEBUG

MAKE_ENUM_INFO_HEADER(InstanceType)
#define MAKE_ENTRY(n, NAME, Name, info) MAKE_ENUM_INFO_ENTRY(t##Name)
FOR_EACH_DECLARED_TYPE(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

MAKE_ENUM_INFO_HEADER(Signal::Type)
#define MAKE_ENTRY(n, NAME, Name, info) MAKE_ENUM_INFO_ENTRY(Signal::s##Name)
FOR_EACH_SIGNAL_TYPE(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

const char *Layout::layout_name(uword tag) {
  switch (tag) {
#define MAKE_TYPE_CASE(n, NAME, Name, info) case t##Name: return #Name;
FOR_EACH_DECLARED_TYPE(MAKE_TYPE_CASE)
#undef MAKE_TYPE_CASE
    default: return "<illegal>";
  }
}

// -----------------------
// --- P r i n t i n g ---
// -----------------------

string Data::to_string(Data::WriteMode mode) {
  string_buffer buf;
  write_on(buf, mode);
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
  for (uword i = 0; i < obj->length(); i++)
    buf.append(obj->at(i));
  if (mode != Data::UNQUOTED) buf.append('"');
}

static void write_protocol_short_on(Protocol *obj, string_buffer &buf) {
  buf.append("#<protocol ");
  obj->name()->write_on(buf, Data::UNQUOTED);
  buf.append(">");
}

static void write_layout_short_on(Layout *obj, string_buffer &buf) {
  buf.append("#<layout");
  Immediate *protocol = obj->protocol();
  if (is<Protocol>(protocol)) {
    buf.append(" ");
    cast<Protocol>(protocol)->name()->write_on(buf, Data::UNQUOTED);
  }
  buf.append(">");
}

static void write_instance_short_on(Instance *obj, string_buffer &buf) {
  Immediate *protocol = obj->layout()->protocol();
  if (is<Protocol>(protocol)) {
    Value *layout_name = cast<Protocol>(protocol)->name();
    if (is<String>(layout_name)) {
      buf.append("#<");
      if (cast<String>(layout_name)->starts_with_vowel()) {
        buf.append("an ");
      } else {
        buf.append("a ");
      }
      layout_name->write_on(buf, Data::UNQUOTED);
      buf.append(">");
      return;
    }
  }
  buf.append("#<instance>");
}

static void write_syntax_tree_on(SyntaxTree *obj, string_buffer &buf) {
  buf.append("«");
  obj->unparse_on(buf);
  buf.append("»");
}

static void write_selector_short_on(Selector *obj, string_buffer &buf) {
  obj->name()->write_short_on(buf, Data::UNQUOTED);
  if (!is<True>(obj->is_accessor())) {
    buf.append('(');
    obj->argc()->write_short_on(buf);
    Tuple *keywords = obj->keywords();
    if (!keywords->is_empty()) {
      buf.append(" |");
      for (uword i = 0; i < keywords->length(); i++) {
        buf.append(" ");
        Immediate *key = cast<Immediate>(keywords->get(i));
        key->write_short_on(buf, Data::UNQUOTED);
        buf.append(":");
      }
    }
    buf.append(')');
  }
}

static void write_channel_short_on(Channel *obj, string_buffer &buf) {
  buf.append("#<channel ");
  obj->name()->write_short_on(buf, Data::UNQUOTED);
  buf.append(">");
}

static void write_object_short_on(Object *obj, Data::WriteMode mode, string_buffer &buf) {
  uword instance_type = obj->layout()->instance_type();
  switch (instance_type) {
  case tString:
    write_string_short_on(cast<String>(obj), mode, buf);
    break;
  case tTuple:
    buf.append("#<tuple>");
    break;
  case tLambda:
    buf.append("#<lambda>");
    break;
  case tTask:
    buf.append("#<task>");
    break;
  case tVoid:
    buf.append("void");
    break;
  case tNull:
    buf.append("null");
    break;
  case tTrue:
    buf.append("true");
    break;
  case tFalse:
    buf.append("false");
    break;
  case tDictionary:
    buf.append("#<dictionary>");
    break;
  case tCode:
    buf.append("#<code>");
    break;
  case tProtocol:
    write_protocol_short_on(cast<Protocol>(obj), buf);
    break;
  case tLayout:
    write_layout_short_on(cast<Layout>(obj), buf);
    break;
  case tSelector:
    write_selector_short_on(cast<Selector>(obj), buf);
    break;
  case tMethod:
    buf.append("#<method>");
    break;
  case tChannel:
    write_channel_short_on(cast<Channel>(obj), buf);
    break;
  case tInstance:
    write_instance_short_on(cast<Instance>(obj), buf);
    break;
#define MAKE_CASE(n, NAME, Name, info) case t##Name:
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
  case Signal::sNothing:
    buf.append("@<nothing>");
    break;
  case Signal::sAllocationFailed:
    buf.append("@<allocation failed>");
    break;
  case Signal::sInternalError:
    buf.append("@<internal error>");
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
  } else if (is<Forwarder>(obj)) {
    write_data_short_on(cast<Forwarder>(obj)->descriptor()->target(), mode, buf);
  } else {
    UNREACHABLE();
  }
}

static void write_tuple_on(Tuple *obj, string_buffer &buf) {
  buf.append('[');
  for (uword i = 0; i < obj->length(); i++) {
    if (i > 0) buf.append(", ");
    obj->get(i)->write_short_on(buf);
  }
  buf.append(']');
}

static void write_lambda_on(Lambda *obj, string_buffer &buf) {
  if (is<SyntaxTree>(obj->tree()))
    cast<SyntaxTree>(obj->tree())->unparse_on(buf);
  else
    buf.append("#<lambda>");
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
  obj->selector()->write_short_on(buf);
}

static void write_object_on(Object *obj, Data::WriteMode mode, string_buffer &buf) {
  switch (obj->type()) {
  case tTuple:
    write_tuple_on(cast<Tuple>(obj), buf);
    break;
  case tLambda:
    write_lambda_on(cast<Lambda>(obj), buf);
    break;
  case tDictionary:
    write_dictionary_on(cast<Dictionary>(obj), buf);
    break;
  case tMethod:
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

static void disassemble_buffer(uint16_t *data, uword size,
    Tuple *literals, string_buffer &buf) {
  uword pc = 0;
  while (pc < size) {
    buf.printf("%{ 3} ", pc);
    switch (data[pc]) {
      case OC_ARGUMENT:
        buf.printf("argument %", data[pc + 1]);
        pc += OpcodeInfo<OC_ARGUMENT>::kSize;
        break;
      case OC_LD_LOCAL:
        buf.printf("load local %", data[pc + 1]);
        pc += OpcodeInfo<OC_LD_LOCAL>::kSize;
        break;
      case OC_GLOBAL: {
        scoped_string name(literals->get(data[pc + 1])->to_string());
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
        scoped_string value(literals->get(data[pc + 1])->to_string());
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
        scoped_string name(literals->get(data[pc + 1])->to_string());
        buf.printf("invoke %", name.chars());
        pc += OpcodeInfo<OC_INVOKE>::kSize;
        break;
      }
      case OC_RAISE: {
        scoped_string name(literals->get(data[pc + 1])->to_string());
        buf.printf("raise %", name.chars());
        pc += OpcodeInfo<OC_RAISE>::kSize;
        break;
      }
      case OC_NEW: {
        buf.append("new");
        pc += OpcodeInfo<OC_NEW>::kSize;
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
      case OC_TUPLE:
        buf.printf("tuple %", data[pc + 1]);
        pc += OpcodeInfo<OC_TUPLE>::kSize;
        break;
      default:
        UNHANDLED(Opcode, data[pc]);
        return;
    }
    buf.append("\n");
  }
}

string Lambda::disassemble() {
  uword size = cast<Code>(code())->length();
  uint16_t *data = &cast<Code>(code())->at(0);
  string_buffer buf;
  disassemble_buffer(data, size, cast<Tuple>(constant_pool()), buf);
  return buf.to_string();
}


// ---------------
// --- S i z e ---
// ---------------

uword Object::size_in_memory() {
  InstanceType instance_type = type();
  switch (instance_type) {
  case tTrue: case tFalse: case tVoid: case tNull:
    return Singleton::kSize;
  case tLambda:
    return Lambda::kSize;
  case tBuiltinCall:
    return BuiltinCall::kSize;
  case tLayout:
    return Layout::kSize;
  case tContext:
    return Context::kSize;
  case tChannel:
    return Channel::kSize;
  case tTuple:
    return Tuple::size_for(cast<Tuple>(this)->length());
  case tString:
    return String::size_for(cast<String>(this)->length());
  case tStack:
    return Stack::size_for(cast<Stack>(this)->height());
  case tCode: case tBuffer:
    return AbstractBuffer::size_for(cast<AbstractBuffer>(this)->size<uint8_t>());
#define MAKE_CASE(n, NAME, Name, name) case t##Name: return Name::kSize;
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
  for (uword i = 0; i < obj->length(); i++)
    GC_SAFE_CHECK_IS_C(VALIDATION, Value, obj->get(i));
}

static void validate_instance(Instance *obj) {
  for (uword i = 0; i < obj->gc_safe_layout()->instance_field_count(); i++)
    GC_SAFE_CHECK_IS_C(VALIDATION, Value, obj->get_field(i));
}

#define VALIDATE_FIELD(Type, name, Name, Class)                      \
  GC_SAFE_CHECK_IS_C(VALIDATION, Type, cast<Class>(obj)->name());

static void validate_object(Object *obj) {
  GC_SAFE_CHECK_IS_C(VALIDATION, Layout, obj->layout());
  InstanceType type = obj->gc_safe_type();
  switch (type) {
    case tTuple:
      validate_tuple(cast<Tuple>(obj));
      break;
    case tInstance:
      validate_instance(cast<Instance>(obj));
      break;
    case tStack:
      cast<Stack>(obj)->validate_stack();
      break;
    case tLambda:
      FOR_EACH_LAMBDA_FIELD(VALIDATE_FIELD, Lambda)
      if (!is<Smi>(cast<Lambda>(obj)->code())) {
        GC_SAFE_CHECK_IS_C(VALIDATION, Code, cast<Lambda>(obj)->code());
        GC_SAFE_CHECK_IS_C(VALIDATION, Tuple, cast<Lambda>(obj)->constant_pool());
      }
      break;
    case tChannel:
      FOR_EACH_CHANNEL_FIELD(VALIDATE_FIELD, Channel)
      break;
    case tLayout:
      FOR_EACH_LAYOUT_FIELD(VALIDATE_FIELD, Layout)
      break;
    case tContext:
      FOR_EACH_CONTEXT_FIELD(VALIDATE_FIELD, Context)
      break;
    case tQuoteTemplate:
      FOR_EACH_QUOTE_TEMPLATE_FIELD(VALIDATE_FIELD, QuoteTemplate)
      break;
    case tBuiltinCall: case tUnquoteExpression: case tCode:
    case tString: case tVoid: case tTrue: case tFalse: case tNull:
    case tBuffer:
      break;
#define MAKE_CASE(n, NAME, Name, name)                               \
    case t##Name:                                                    \
      FOR_EACH_##NAME##_FIELD(VALIDATE_FIELD, Name);                 \
      break;
FOR_EACH_GENERATABLE_TYPE(MAKE_CASE)
#undef MAKE_CASE
    default:
      UNHANDLED(InstanceType, type);
  }
}

#undef VALIDATE_FIELD

bool Immediate::validate() {
  if (is<Object>(this)) validate_object(cast<Object>(this));
  return true;
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
    case tString: case tCode: case tTrue: case tFalse: case tVoid:
    case tNull: case tContext:
      break;
    case tTuple:
      for (uword i = 0; i < cast<Tuple>(this)->length(); i++)
        VISIT(cast<Tuple>(this)->get(i));
      break;
    case tBuiltinCall:
      FOR_EACH_BUILTIN_CALL_FIELD(VISIT_FIELD, BuiltinCall);
      break;
    case tLambda:
      FOR_EACH_LAMBDA_FIELD(VISIT_FIELD, Lambda)
      break;
    case tLayout:
      FOR_EACH_LAYOUT_FIELD(VISIT_FIELD, Layout)
      break;
    case tChannel:
      FOR_EACH_CHANNEL_FIELD(VISIT_FIELD, Channel)
      break;
    case tStack:
      cast<Stack>(this)->for_each_stack_field(visitor);
      break;
#define MAKE_CASE(n, NAME, Name, name)                               \
    case t##Name:                                                    \
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
  return heap.new_lambda(argc(), code(), constant_pool(), tree(), context());
}


// -----------------------
// --- E q u a l i t y ---
// -----------------------

bool Value::is_key() {
  if (is_atomic_key()) return true;
  switch (type()) {
  case tSelector: case tTuple:
    return true;
  default:
    return false;
  }
}

bool Value::is_atomic_key() {
  switch (type()) {
    case tString: case tVoid: case tNull: case tTrue: case tFalse:
    case tSmi: case tSymbol:
      return true;
    default:
      return false;
  }
}

bool Value::is_identical(Value *that) {
  if (this == that) return true;
  if (this->type() != that->type()) return false;
  if (this->is_atomic_key()) return this->equals(that);
  else return false;
}

bool Value::equals(Value *that) {
  ASSERT(this->is_key());
  ASSERT(that->is_key());
  if (this == that) return true;
  if (this->type() != that->type()) return false;
  InstanceType type = this->type();
  switch (type) {
  case tString:
    return cast<String>(this)->string_equals(cast<String>(that));
  case tSelector:
    return cast<Selector>(this)->selector_equals(cast<Selector>(that));
  case tTuple:
    return cast<Tuple>(this)->tuple_equals(cast<Tuple>(that));
  case tSmi: case tSymbol:
    return this == that;
  case tVoid:
    return is<Void>(that);
  case tNull:
    return is<Null>(that);
  case tTrue:
    return is<True>(that);
  case tFalse:
    return is<False>(that);
  default:
    UNHANDLED(InstanceType, type);
    return false;
  }
}

bool String::string_equals(String *that) {
  if (this->length() != that->length())
    return false;
  for (uword i = 0; i < this->length(); i++) {
    if (this->at(i) != that->at(i))
      return false;
  }
  return true;
}

bool Tuple::tuple_equals(Tuple *that) {
  if (this->length() != that->length())
    return false;
  for (uword i = 0; i < this->length(); i++) {
    if (!this->get(i)->equals(that->get(i)))
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
  uword chr = at(0);
  if ('A' <= chr && chr <= 'Z') chr += 'a' - 'A';
  switch (chr) {
  case 'a': case 'e': case 'i': case 'o': case 'u':
    return true;
  default:
    return false;
  }
}

vector<char> String::c_str() {
  uword length = this->length();
  vector<char> result = ALLOCATE_VECTOR(char, length + 1);
  for (uword i = 0; i < length; i++)
    result[i] = at(i);
  result[length] = '\0';
  return result;
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
  for (uword i = 0; i < table->length(); i += 2) {
    if (key->equals(table->get(i))) {
      result.value = &table->get(i + 1);
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
    uword length = table->length();
    Data *new_table_val = Runtime::current().heap().new_tuple(length + 2);
    if (is<AllocationFailed>(new_table_val)) return false;
    Tuple *new_table = cast<Tuple>(new_table_val);
    for (uword i = 0; i < length; i++)
      new_table->set(i, table->get(i));
    new_table->set(length, key);
    new_table->set(length + 1, value);
    this->set_table(new_table);
  }
  return true;
}

uword Dictionary::size() {
  return table()->length() / 2;
}


// -----------------
// --- C l a s s ---
// -----------------

bool Layout::is_empty() {
  return protocol() == Smi::from_int(0);
}

Data *Layout::clone(Heap &heap) {
  return heap.new_layout(instance_type(), instance_field_count(),
      protocol(), methods());
}


} // namespace neutrino
