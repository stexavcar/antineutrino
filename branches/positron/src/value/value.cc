#include "runtime/heap-inl.h"
#include "value/value-inl.h"
#include "value/condition-inl.h"

namespace neutrino {

#define DECLARE_VIRTUALS(__Name__, __Class__, __Species__)           \
  Virtuals __Name__ = {                                              \
    {                                                                \
      __Class__::clone_object<__Class__>,                            \
      __Class__::object_size_in_memory<__Class__>,                   \
      __Class__::migrate_object_fields<__Class__>                    \
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

DataFormatter DataFormatter::kInstance;

void DataFormatter::print_on(const variant &that, string modifiers,
      string_stream &stream) {
  Data *obj = static_cast<Data*>(const_cast<void*>(that.data_.u_ptr));
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
  if (memory.is_empty()) return InternalError::make(InternalError::ieHeapExhaustion);
  return new (memory) T(*old);
}

template <typename T>
word Object::object_size_in_memory(Species *species, Object *obj) {
  return sizeof(T);
}

template <typename T>
static word object_size_in_memory(Species *desc, Object *obj);

template <typename T>
static void migrate_object_fields(Species *desc, Object *obj,
    FieldMigrator &migrator);


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
  if (memory.is_empty()) return InternalError::make(InternalError::ieHeapExhaustion);
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

DECLARE_VIRTUALS(Array::kVirtuals, Array, Species);

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
  if (memory.is_empty()) return InternalError::make(InternalError::ieHeapExhaustion);
  String *result = new (memory) String(*old);
  array<code_point> from = old->chars();
  array<code_point> to = result->chars();
  from.copy_to(to, length);
  return result;
}

bool String::equals(const string &str) {
  if (length() != str.length()) return false;
  for (word i = 0; i < length(); i++) {
    if (chars()[i] != static_cast<code_point>(str[i]))
      return false;
  }
  return true;
}

DECLARE_VIRTUALS(String::kVirtuals, String, Species);

} // namespace neutrino
