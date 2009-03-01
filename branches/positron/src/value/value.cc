#include "runtime/heap-inl.h"
#include "value/value-inl.h"
#include "value/condition-inl.h"

namespace neutrino {

#define DECLARE_VIRTUALS(__Name__, __Class__, __Species__)           \
  Virtuals __Name__ = {                                              \
    {                                                                \
      __Class__::clone_object<__Class__>,                            \
      __Class__::object_size_in_memory<__Class__>,                   \
      __Class__::migrate_object_fields<__Class__>,                   \
      __Class__::object_hash<__Class__>                              \
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

template <typename T>
word Object::object_hash(Object *obj) {
  return cast<T>(obj)->calculate_hash();
}

word Object::calculate_hash() {
  assert false;
  return 0;
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

word String::calculate_hash() {
  vector<code_point> str = as_vector();
  word hash = 0;
  uword rotand = 0;
  for (word i = 0; i < str.length(); i++) {
    uword c = str[i];
    rotand ^= c & ((8 * sizeof(word)) - 1);
    hash = ((hash << rotand) | (hash >> rotand)) ^ c;
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
  return map()->table()->elements() + (kSize * index());
}

bool HashMap::Entry::is_occupied() {
  return is<TaggedInteger>(hash());
}

Value *&HashMap::Entry::key() {
  return entries()[kKeyOffset];
}

Value *&HashMap::Entry::value() {
  return entries()[kValueOffset];
}

Value *&HashMap::Entry::hash() {
  return entries()[kHashOffset];
}

void HashMap::migrate_fields(FieldMigrator &migrator) {
  Object::migrate_fields(migrator);
  migrator.migrate_field(reinterpret_cast<Value**>(&table_));
}

possibly HashMap::set(Value *key, Value *value) {
  word hash_value = key->hash() % TaggedInteger::kUpperLimit;
  TaggedInteger *hash = TaggedInteger::make(hash_value);
  Entry entry = lookup(hash, key);
  if (entry.is_occupied()) {
    assert false;
  } else {
    entry.key() = key;
    entry.hash() = hash;
    entry.value() = value;
  }
  return Success::make();
}

HashMap::Entry HashMap::lookup(TaggedInteger *hash, Value *key) {
  vector<Value*> table = this->table()->as_vector();
  word end = table.length() / Entry::kSize;
  word pos = hash->value() % end;
  Entry current(this, pos);
  while (current.is_occupied()) {
    if (current.hash() == hash && current.key() == key)
      break;
    pos = (pos + 1) % end;
    current = Entry(this, pos);
  }
  return current;
}

/* --- P r i n t i n g --- */

void DataFormatter::print_on(const variant &that, string modifiers,
      string_stream &stream) {
  Data *obj = static_cast<Data*>(const_cast<void*>(that.data_.u_ptr));
  if (is<Signal>(obj)) {
    switch (cast<Signal>(obj)->type()) {
#define MAKE_CASE(Name)                                              \
      case Signal::s##Name: cast<Name>(obj)->print_on(modifiers, stream); \
      return;
    eSignalTypes(MAKE_CASE)
#undef MAKE_CASE
    default:
      break;
    }
  }
  if (is<String>(obj)) {
    String *str = cast<String>(obj);
    array<code_point> chars = str->chars();
    bool quote = modifiers.contains('q');
    if (quote) stream.add('"');
    for (word i = 0; i < str->length(); i++)
      stream.add(chars[i]);
    if (quote) stream.add('"');
  } else {
    stream.add("[todo]");
  }
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
