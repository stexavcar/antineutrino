#include "runtime/heap-inl.h"
#include "value/value-inl.h"
#include "value/condition-inl.h"

namespace neutrino {

template <typename T>
static word object_hash_bridge(Object *obj) {
  return cast<T>(obj)->hash();
}

template <typename T>
static bool object_equals_bridge(Object *obj, Value *that) {
  return cast<T>(obj)->equals(that);
}

#define DECLARE_VIRTUALS(__Name__, __Class__, __Species__)           \
  Virtuals __Name__ = {                                              \
    {                                                                \
      __Class__::clone_object<__Class__>,                            \
      __Class__::object_size_in_memory<__Class__>,                   \
      __Class__::migrate_object_fields<__Class__>,                   \
      object_hash_bridge<__Class__>,                                 \
      object_equals_bridge<__Class__>                                \
    }, {                                                             \
      __Species__::clone_species<__Species__>,                       \
      __Species__::species_size_in_memory<__Species__>,              \
      __Species__::migrate_species_fields<__Species__>               \
    }                                                                \
  }

/* --- D a t a --- */

Data::DataType Data::type() {
  if (is<Signal>(this)) {
    return dtSignal;
  } else if (is<Object>(this)) {
    return static_cast<DataType>(cast<Object>(this)->type());
  } else {
    return dtUnknown;
  }
}

void Data::println() {
  string_stream stream;
  stream.add("%", vargs(this));
  stream.raw_string().println();
}

DataFormatter DataFormatter::kInstance;

/* --- V a l u e --- */

word Value::hash() {
  if (is<Object>(this)) {
    Object *obj = cast<Object>(this);
    return obj->species()->virtuals().object.hash(obj);
  } else {
    assert false;
    return 0;
  }
}

bool Value::equals(Value *that) {
  if (this == that) return true;
  if (is<Object>(this)) {
    Object *obj = cast<Object>(this);
    return obj->species()->virtuals().object.equals(obj, that);
  } else {
    return false;
  }
}

/* --- O b j e c t --- */

template <typename T>
void Object::migrate_object_fields(Species *desc, Object *obj,
    FieldMigrator &migrator) {
  return static_cast<T*>(obj)->migrate_fields(migrator);
}

void Object::migrate_fields(FieldMigrator &migrator) {
  migrator.migrate_field(reinterpret_cast<Value**>(&header()));
}

template <typename T>
allocation<Object> Object::clone_object(Species *desc, Object *obj,
    Space &space) {
  T *old = cast<T>(obj);
  word size = desc->virtuals().object.size_in_memory(desc, obj);
  array<uint8_t> memory = space.allocate(size);
  if (memory.is_empty()) return InternalError::heap_exhaustion();
  return new (memory) T(*old);
}

template <typename T>
word Object::object_size_in_memory(Species *species, Object *obj) {
  return sizeof(T);
}

word Object::hash() {
  assert false;
  return 0;
}

bool Object::equals(Value *that) {
  return this == that;
}

/* --- S p e c i e s --- */

template <typename T>
word Species::object_size_in_memory(Species *desc, Object *obj) {
  Species *that = static_cast<Species*>(obj);
  return that->virtuals().species.size_in_memory(that);
}

template <typename T>
allocation<Object> Species::clone_object(Species *desc, Object *obj,
    Space &space) {
  Species *that = static_cast<Species*>(obj);
  return that->virtuals().species.clone(that, space);
}

template <typename T>
void Species::migrate_object_fields(Species *desc, Object *obj,
    FieldMigrator &migrator) {
  Species *that = static_cast<Species*>(obj);
  return that->virtuals().species.migrate_fields(that, migrator);
}

template <typename T>
allocation<Species> Species::clone_species(Species *that,
    Space &space) {
  pTryAllocInSpace(space, T, result, (*static_cast<T*>(that)));
  return result;
}

template <typename T>
word Species::species_size_in_memory(Species *desc) {
  return sizeof(T);
}

template <typename T>
void Species::migrate_species_fields(Species *desc,
    FieldMigrator &migrator) {
  return static_cast<T*>(desc)->migrate_fields(migrator);
}

DECLARE_VIRTUALS(Species::kSpeciesVirtuals, Species, Species);

/* --- A r r a y --- */

template <typename T>
word Array::object_size_in_memory(Species *desc, Object *obj) {
  return Array::size_in_memory(cast<Array>(obj)->length());
}

template <typename T>
allocation<Object> Array::clone_object(Species *species, Object *obj,
    Space &space) {
  Array *old = cast<Array>(obj);
  word length = old->length();
  word size = Array::size_in_memory(length);
  array<uint8_t> memory = space.allocate(size);
  if (memory.is_empty()) return InternalError::heap_exhaustion();
  Array *result = new (memory) Array(*old);
  array<Value*> from = old->elements();
  array<Value*> to = result->elements();
  from.copy_to(to, length);
  return result;
}

void Array::migrate_fields(FieldMigrator &migrator) {
  Object::migrate_fields(migrator);
  word n = length();
  array<Value*> elms = elements();
  for (word i = 0; i < n; i++)
    migrator.migrate_field(&elms[i]);
}

#define DECLARE_SIMPLE_VIRTUALS(Name) DECLARE_VIRTUALS(Name::kVirtuals, Name, Species);
eSimpleConcreteValueTypes(DECLARE_SIMPLE_VIRTUALS)
#undef DECLARE_SIMPLE_VIRTUALS

void SyntaxTree::migrate_fields(FieldMigrator &migrator) {
  Object::migrate_fields(migrator);
  migrator.migrate_field(reinterpret_cast<Value**>(&code_));
  migrator.migrate_field(reinterpret_cast<Value**>(&literals_));
}

/* --- S t r i n g --- */

template <typename T>
word String::object_size_in_memory(Species *desc, Object *obj) {
  return String::size_in_memory(cast<String>(obj)->length());
}

template <typename T>
allocation<Object> String::clone_object(Species *desc, Object *obj,
    Space &space) {
  String *old = cast<String>(obj);
  word length = old->length();
  word size = String::size_in_memory(length);
  array<uint8_t> memory = space.allocate(size);
  if (memory.is_empty()) return InternalError::heap_exhaustion();
  String *result = new (memory) String(*old);
  array<code_point> from = old->chars();
  array<code_point> to = result->chars();
  from.copy_to(to, length);
  return result;
}

word String::hash() {
  static const word kBitCount = (8 * sizeof(word));
  vector<code_point> str = as_vector();
  word hash = str.length();
  uword rotand = 0;
  for (word i = 0; i < str.length(); i++) {
    uword c = str[i];
    rotand = (rotand + c) & (kBitCount - 1);
    hash = ((hash << rotand) | (static_cast<uword>(hash) >> (kBitCount - rotand))) ^ c;
  }
  return hash;
}

bool String::equals(const string &str) {
  if (length() != str.length()) return false;
  for (word i = 0; i < length(); i++) {
    if (chars()[i] != static_cast<code_point>(str[i]))
      return false;
  }
  return true;
}

bool String::equals(Value *other) {
  if (this == other)
    return true;
  if (!is<String>(other))
    return false;
  String *that = cast<String>(other);
  word length = this->length();
  if (that->length() != length)
    return false;
  for (word i = 0; i < length; i++) {
    if (chars()[i] != that->chars()[i])
      return false;
  }
  return true;
}

/* --- B l o b --- */

template <typename T>
word Blob::object_size_in_memory(Species *desc, Object *obj) {
  return Blob::size_in_memory(cast<Blob>(obj)->length<uint8_t>());
}

template <typename T>
allocation<Object> Blob::clone_object(Species *desc, Object *obj,
    Space &space) {
  Blob *old = cast<Blob>(obj);
  word length = old->length<uint8_t>();
  word size = Blob::size_in_memory(length);
  array<uint8_t> memory = space.allocate(size);
  if (memory.is_empty()) return InternalError::heap_exhaustion();
  Blob *result = new (memory) Blob(*old);
  array<uint8_t> from = old->data<uint8_t>();
  array<uint8_t> to = result->data<uint8_t>();
  from.copy_to(to, length);
  return result;
}

/* --- H a s h   M a p --- */

array<Value*> HashMap::Entry::entries() {
  return table()->elements() + (kSize * index());
}

HashMap::Entry::Entry(Array *table, word index)
  : table_(table), index_(index) {
  assert index >= 0;
}

bool HashMap::Entry::is_occupied() {
  return is<TaggedInteger>(hash());
}

Value *HashMap::Entry::key() {
  return entries()[kKeyOffset];
}

Value *HashMap::Entry::value() {
  return entries()[kValueOffset];
}

Value *HashMap::Entry::hash() {
  return entries()[kHashOffset];
}

void HashMap::Entry::grab(Value *key, Value *value, TaggedInteger *hash) {
  entries()[kKeyOffset] = key;
  entries()[kValueOffset] = value;
  entries()[kHashOffset] = hash;
}

void HashMap::migrate_fields(FieldMigrator &migrator) {
  Object::migrate_fields(migrator);
  migrator.migrate_field(reinterpret_cast<Value**>(&table_));
}

word HashMap::capacity() {
  return table()->length() / Entry::kSize;
}

possibly HashMap::extend_capacity(Heap &heap, word new_capacity) {
  word old_capacity = capacity();
  assert new_capacity > old_capacity;
  try alloc Array *new_table = heap.new_array(new_capacity * Entry::kSize);
  Array *old_table = table();
  table_ = new_table;
  for (word i = 0; i < old_capacity; i++) {
    Entry old_entry(old_table, i);
    if (!old_entry.is_occupied()) continue;
    Value *key = old_entry.key();
    TaggedInteger *hash = cast<TaggedInteger>(old_entry.hash());
    Entry new_entry = lookup(hash, key);
    assert !new_entry.is_occupied();
    new_entry.grab(key, old_entry.value(), hash);
  }
  return Success::make();
}

static TaggedInteger *hash_value(Value *key) {
  word value = key->hash() % TaggedInteger::kUpperLimit;
  return TaggedInteger::make(value);
}

possibly HashMap::set(Heap &heap, Value *key, Value *value) {
  if (size() * 100 > capacity() * kLoadFactorPercent)
    try extend_capacity(heap, grow_value(capacity()));
  TaggedInteger *hash = hash_value(key);
  Entry entry = lookup(hash, key);
  if (!entry.is_occupied()) size_++;
  entry.grab(key, value, hash);
  return Success::make();
}

Data *HashMap::get(Value *key) {
  Entry entry = lookup(hash_value(key), key);
  if (entry.is_occupied()) {
    return entry.value();
  } else {
    return InternalError::missing();
  }
}

HashMap::Entry HashMap::lookup(TaggedInteger *hash, Value *key) {
  Array *array = table();
  vector<Value*> table = array->as_vector();
  word end = capacity();
  word pos = static_cast<uword>(hash->value()) % end;
  Entry current(array, pos);
  while (current.is_occupied()) {
    if (current.hash() == hash && current.key()->equals(key))
      break;
    pos = (pos + 1) % end;
    current = Entry(array, pos);
  }
  return current;
}

bool HashMap::Iterator::next(Value **key_out, Value **value_out) {
  while (offset_ < map()->capacity()) {
    HashMap::Entry entry(map()->table(), offset_++);
    if (entry.is_occupied()) {
      *key_out = entry.key();
      *value_out = entry.value();
      return true;
    }
  }
  return false;
}

/* --- P r i n t i n g --- */

void DataFormatter::print_on(const variant &that, string modifiers,
      string_stream &stream) {
  Data *obj = static_cast<Data*>(const_cast<void*>(that.data_.u_ptr));
  if (is<Signal>(obj)) {
    switch (cast<Signal>(obj)->type()) {
#define MAKE_CASE(Name)                                              \
      case Signal::s##Name:                                          \
        cast<Name>(obj)->print_on(modifiers, stream);                \
        return;
    eSignalTypes(MAKE_CASE)
#undef MAKE_CASE
    default:
      break;
    }
  } else if (is<Object>(obj)) {
    switch (cast<Object>(obj)->type()) {
#define MAKE_CASE(Name)                                              \
      case Value::t##Name:                                           \
        cast<Name>(obj)->print_on(modifiers, stream);                \
        return;
      eConcreteValueTypes(MAKE_CASE)
#undef MAKE_CASE
      default:
        break;
    }
  } else if (is<TaggedInteger>(obj)) {
    stream.add("%", vargs(cast<TaggedInteger>(obj)->value()));
  } else {
    stream.add("#<unknown>");
  }
}

void Object::print_on(string modifiers, string_stream &out) {
  out.add("#<an Object>");
}

void String::print_on(string modifiers, string_stream &out) {
  vector<code_point> chars = as_vector();
  bool quote = modifiers.contains('q');
  if (quote) out.add('"');
  for (word i = 0; i < chars.length(); i++)
    out.add(chars[i]);
  if (quote) out.add('"');
}

void HashMap::print_on(string modifiers, string_stream &out) {
  out.add('{');
  HashMap::Iterator iter(this);
  Value *key = NULL;
  Value *value = NULL;
  bool first = true;
  while (iter.next(&key, &value)) {
    if (first) first = false;
    else out.add(", ");
    out.add("%{q}: %{q}", vargs(key, value));
  }
  out.add('}');
}

void FatalError::print_on(string modifiers, string_stream &out) {
  string type;
  switch (this->error()) {
#define MAKE_CASE(Name, name)                                        \
    case FatalError::fe##Name:                                       \
      type = #name;                                                  \
      break;
    eFatalErrorTypes(MAKE_CASE)
#undef MAKE_CASE
    default:
      type = "[unknown]";
      break;
  }
  out.add("@@<fatal error: %>", vargs(type));
}

void InternalError::print_on(string modifiers, string_stream &out) {
  out.add("@@<internal error>");
}

void Success::print_on(string modifiers, string_stream &out) {
  out.add("@@<success>");
}

} // namespace neutrino
