#include "compiler/ast-inl.h"
#include "heap/heap.h"
#include "heap/ref-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/hash-inl.h"
#include "utils/string-inl.h"
#include "values/values-inl.h"

namespace neutrino {

#ifdef DEBUG

uword Layout::tag_of(Data *value) {
  if (is<Signal>(value)) {
    switch (cast<Signal>(value)->type()) {
#define MAKE_SIGNAL_TYPE_CASE(n, Name, info) case Signal::s##Name: return t##Name;
eSignalTypes(MAKE_SIGNAL_TYPE_CASE)
#undef MAKE_SIGNAL_TYPE_CASE
      default: UNREACHABLE(); return tSignal;
    }
  } else {
    return cast<Value>(value)->gc_safe_type();
  }
}

#endif // DEBUG

string Layout::name_for(InstanceType type) {
  switch (type) {
#define MAKE_TYPE_CASE(n, Name, info) case t##Name: return #Name;
eDeclaredTypes(MAKE_TYPE_CASE)
#undef MAKE_TYPE_CASE
    default: return "<illegal>";
  }
}

MAKE_ENUM_INFO_HEADER(InstanceType)
#define MAKE_ENTRY(n, Name, info) MAKE_ENUM_INFO_ENTRY(t##Name)
eDeclaredTypes(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

MAKE_ENUM_INFO_HEADER(Signal::Type)
#define MAKE_ENTRY(n, Name, info) MAKE_ENUM_INFO_ENTRY(Signal::s##Name)
eSignalTypes(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

const char *Layout::layout_name(uword tag) {
  switch (tag) {
#define MAKE_TYPE_CASE(n, Name, info) case t##Name: return #Name;
eDeclaredTypes(MAKE_TYPE_CASE)
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
  buf.printf("%", elms(obj->value()));
}

static void write_string_short_on(String *obj, Data::WriteMode mode, string_buffer &buf) {
  if (mode != Data::UNQUOTED) buf.append('"');
  for (uword i = 0; i < obj->length(); i++)
    buf.append(obj->get(i));
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
  case tArray:
    buf.append("#<array>");
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
  case tHashMap:
    buf.append("#<hash_map>");
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
  case tSymbol:
    buf.printf("#<symbol %>", elms(cast<Symbol>(obj)->name()));
    break;
#define MAKE_CASE(n, Name, info) case t##Name:
eSyntaxTreeTypes(MAKE_CASE)
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
  case Signal::sStackOverflow: {
    uword height = cast<StackOverflow>(obj)->height();
    buf.printf("@<stack overflow: %>", elms(height));
    break;
  }
  case Signal::sTypeMismatch: {
    string expected = Layout::name_for(cast<TypeMismatch>(obj)->expected());
    string found = Layout::name_for(cast<TypeMismatch>(obj)->found());
    buf.printf("@<type mismatch: found % expected %>", elms(found, expected));
    break;
  }
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

static void write_hash_map_on(HashMap *obj, string_buffer &buf) {
  buf.append('{');
  HashMap::Iterator iter(obj);
  HashMap::Iterator::Entry entry;
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
  case tHashMap:
    write_hash_map_on(cast<HashMap>(obj), buf);
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


// ---------------
// --- S i z e ---
// ---------------

uword Object::size_in_memory() {
  InstanceType instance_type = type();
  switch (instance_type) {
  case tTrue: case tFalse: case tVoid: case tNull:
    return Singleton::kSize;
  case tTuple:
    return Tuple::size_for(cast<Tuple>(this)->length());
  case tArray:
    return Array::size_for(cast<Array>(this)->length());
  case tString:
    return String::size_for(cast<String>(this)->length());
  case tStack:
    return Stack::size_for(cast<Stack>(this)->height());
  case tInstance:
    return Instance::size_for(cast<Instance>(this)->layout()->instance_field_count());
  case tCode: case tBuffer:
    return AbstractBuffer::size_for(cast<AbstractBuffer>(this)->size<uint8_t>());
#define MAKE_CASE(n, Name, name) case t##Name: return Name::kSize;
eBoilerplateSizeInHeap(MAKE_CASE)
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

static void validate_abstract_tuple(AbstractTuple *obj) {
  for (uword i = 0; i < obj->length(); i++)
    GC_SAFE_CHECK_IS_C(cnValidation, Value, obj->get(i));
}

static void validate_instance(Instance *obj) {
  for (uword i = 0; i < obj->gc_safe_layout()->instance_field_count(); i++)
    GC_SAFE_CHECK_IS_C(cnValidation, Value, obj->get_field(i));
}

#define VALIDATE_FIELD(Type, name, Name, Class)                      \
  GC_SAFE_CHECK_IS_C(cnValidation, Type, cast<Class>(obj)->name());

static void validate_object(Object *obj) {
  GC_SAFE_CHECK_IS_C(cnValidation, Layout, obj->layout());
  InstanceType type = obj->gc_safe_type();
  switch (type) {
    case tTuple: case tArray:
      validate_abstract_tuple(cast<AbstractTuple>(obj));
      break;
    case tInstance:
      validate_instance(cast<Instance>(obj));
      break;
    case tStack:
      cast<Stack>(obj)->validate_stack();
      break;
    case tLambda:
      eLambdaFields(VALIDATE_FIELD, Lambda)
      if (!is<Smi>(cast<Lambda>(obj)->code())) {
        GC_SAFE_CHECK_IS_C(cnValidation, Code, cast<Lambda>(obj)->code());
        GC_SAFE_CHECK_IS_C(cnValidation, Tuple, cast<Lambda>(obj)->constant_pool());
      }
      break;
    case tBuiltinCall: case tCode: case tBuffer:
    case tString: case tVoid: case tTrue: case tFalse: case tNull:
      break;
#define MAKE_CASE(n, Name, name)                                     \
    case t##Name:                                                    \
      e##Name##Fields(VALIDATE_FIELD, Name);                         \
      break;
eBoilerplateValidate(MAKE_CASE)
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
    case tTuple: case tArray:
      for (uword i = 0; i < cast<AbstractTuple>(this)->length(); i++)
        VISIT(cast<AbstractTuple>(this)->get(i));
      break;
    case tStack:
      cast<Stack>(this)->for_each_stack_field(visitor);
      break;
    case tInstance:
      cast<Instance>(this)->for_each_instance_field(visitor);
      break;
#define MAKE_CASE(n, Name, name)                                     \
    case t##Name:                                                    \
      e##Name##Fields(VISIT_FIELD, Name);                            \
      break;
eBoilerplateIterator(MAKE_CASE)
#undef MAKE_CASE
    default:
      UNHANDLED(InstanceType, type);
  }
}

void Instance::for_each_instance_field(FieldVisitor &visitor) {
  uword count = layout()->instance_field_count();
  for (uword i = 0; i < count; i++)
    visitor.visit_field(&get_field(i));
}

#undef VISIT
#undef VISIT_FIELD

// -------------------
// --- L a m b d a ---
// -------------------

Data *Lambda::clone(Heap &heap) {
  return heap.new_lambda(argc(), max_stack_height(), code(),
      constant_pool(), tree(), context());
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
    if (this->get(i) != that->get(i))
      return false;
  }
  return true;
}

bool AbstractTuple::tuple_equals(Tuple *that) {
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
  uword chr = get(0);
  if ('A' <= chr && chr <= 'Z') chr += 'a' - 'A';
  switch (chr) {
  case 'a': case 'e': case 'i': case 'o': case 'u':
    return true;
  default:
    return false;
  }
}

array<char> String::c_str() {
  uword length = this->length();
  array<char> result = ALLOCATE_ARRAY(char, length + 1);
  for (uword i = 0; i < length; i++)
    result[i] = get(i);
  result[length] = '\0';
  return result;
}


// ---------------------------
// --- D i c t i o n a r y ---
// ---------------------------


class HashMapConfig {
public:
  typedef Tuple *table_t;
  typedef Value *key_t;
  typedef Value *value_t;
  typedef HashMap *map_t;
  typedef Data *raw_table_t;
  typedef Heap &data_t;
  static uword table_length(Tuple *t) { return t->length(); }
  static Value *&table_get(Tuple *t, uword i) { return t->get(i); }
  static void table_set(Tuple *t, uword i, Value *v) { t->set(i, v); }
  static Data *allocate_table(uword s, Heap &h) { return h.new_tuple(s); }
  static void free_table(Tuple *table) { }
  static bool keys_equal(Value *a, Value *b) { return a->equals(b); }
  static Tuple *cast_to_table(Data *d) { return cast<Tuple>(d); }
  static bool is_abort_value(Data *d) { return is<AllocationFailed>(d); }
  static void set_table(HashMap *d, Tuple *t) { d->set_table(t); }
};


Data *HashMap::get(Value *key) {
  HashMapLookup<Value*> lookup;
  if (HashUtils<HashMapConfig>::lookup_entry(this->table(), key, lookup)) return *lookup.value;
  else return Nothing::make();  
}


Data *HashMap::set(Heap &heap, Value *key, Value *value) {
  HashMapLookup<Value*> lookup;
  Data *raw_table = Smi::from_int(0);
  if (HashUtils<HashMapConfig>::ensure_entry(this, this->table(), key, lookup,
      &raw_table, heap)) {
    *lookup.value = value;
    return this;
  } else {
    return raw_table;
  }
}


uword HashMap::size() {
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
