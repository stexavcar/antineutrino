#ifndef _VALUE_VALUE
#define _VALUE_VALUE

#include "utils/array.h"
#include "utils/global.h"
#include "utils/string.h"

namespace neutrino {

class Data {
public:
  enum DataType { dtSignal, dtUnknown, __last_data_type };
  DataType type();
};

inline void encode_variant(variant &that, Data *value);

template <class C>
static inline bool is(Data *data);

template <class C>
static inline C *cast(Data *data);

class Value : public Data {
public:
  enum Type { tSpecies = Data::__last_data_type, tString, tArray, tInstance };
};

template <> struct coerce<Value::Type> { typedef word type; };

/* --- O b j e c t --- */

class Object : public Value {
public:
  inline explicit Object(Species *species);
  inline Type type();
  inline Species *species();
  inline void set_species(Species *v);
  inline void set_forwarding_header(ForwardPointer *v);

  void migrate_fields(FieldMigrator &migrator);

  template <typename T>
  static Object *clone_object(Species *desc, Object *obj);

  template <typename T>
  static word object_size_in_memory(Species *desc, Object *obj);

  template <typename T>
  static void migrate_object_fields(Species *desc, Object *obj,
      FieldMigrator &migrator);

  Data *&header() { return header_; }
private:
  Data *header_;
};

struct Virtuals {
  struct ObjectVirtuals {
    // Clone the given object the given space.
    allocation<Object> (*clone)(Species*, Object*, Space&);
    // Return the size of a given object.
    word (*size_in_memory)(Species*, Object*);
    // Migrate the fields of an object.
    void (*migrate_fields)(Species*, Object*, FieldMigrator&);
  };
  struct SpeciesVirtuals {
    // Clone yourself in the given space.
    allocation<Species> (*clone)(Species*, Space&);
    // Return the size of yourself.
    word (*size_in_memory)(Species*);
    // Migrate your own fields.
    void (*migrate_fields)(Species*, FieldMigrator&);
  };
  ObjectVirtuals object;
  SpeciesVirtuals species;
};

class Species : public Object {
public:
  Species(Species *meta, Type instance_type, Virtuals &virtuals)
    : Object(meta)
    , instance_type_(instance_type)
    , virtuals_(virtuals) { }
  Type instance_type() { return instance_type_; }
  Virtuals &virtuals() { return virtuals_; }
  static Virtuals &species_virtuals() { return kSpeciesVirtuals; }

  template <typename T>
  static allocation<Species> clone_species(Species *desc, Space &space);

  template <typename T>
  static word species_size_in_memory(Species *desc);

  template <typename T>
  static void migrate_species_fields(Species *desc, FieldMigrator &migrator);

  template <typename T>
  static allocation<Object> clone_object(Species *desc, Object *obj, Space &space);

  template <typename T>
  static word object_size_in_memory(Species *desc, Object *obj);

  template <typename T>
  static void migrate_object_fields(Species *desc, Object *obj,
      FieldMigrator &migrator);

private:
  static Virtuals kSpeciesVirtuals;
  Type instance_type_;
  Virtuals &virtuals_;
};

Object::Object(Species *species) : header_(species) { }

/* --- I n s t a n c e --- */

class Instance : public Object {
public:
  static inline word size_in_memory(word fields);
};

class InstanceSpecies : public Species {
public:
  InstanceSpecies(Species *meta, word field_count)
    : Species(meta, tInstance, kVirtuals)
    , field_count_(field_count) { }
  word field_count() { return field_count_; }

  template <typename T>
  static word object_size_in_memory(Species *desc, Object *obj);

private:
  static Virtuals kVirtuals;
  word field_count_;
};

/* --- S t r i n g --- */

class String : public Object {
public:
  String(Species *species, word length)
    : Object(species)
    , length_(length) { }

  inline array<code_point> chars();
  word length() { return length_; }
  bool equals(const string &str);

  template <typename T>
  static allocation<Object> clone_object(Species *desc, Object *obj,
      Space &space);

  template <typename T>
  static word object_size_in_memory(Species *desc, Object *obj);

  static inline word size_in_memory(word chars);

private:
  word length_;
};

class StringSpecies : public Species {
public:
  StringSpecies(Species *meta) : Species(meta, tString, kVirtuals) { }
private:
  static Virtuals kVirtuals;
};

/* --- A r r a y --- */

class Array : public Object {

};

/* --- S i g n a l --- */

class ForwardPointer : public Data {
public:
  inline Object *target();
  static inline ForwardPointer *make(Object *value);
};

class Signal : public Data {
public:
  enum Type { sSuccess, sInternalError, sFatalError };
  inline Type type();
  inline word payload();
};

template <> struct coerce<Signal::Type> { typedef word type; };

class Success : public Signal {
public:
  static inline Success *make();
};

class Failure : public Signal {
};

class InternalError : public Failure {
public:
  enum Type { ieUnknown, ieSystem, ieEnvironment, ieHeapExhaustion };
  static inline InternalError *make(Type type);
};

template <> struct coerce<InternalError::Type> { typedef word type; };

class FatalError : public Failure {
public:
  enum Type { feOutOfMemory, feAbort };
  static inline FatalError *make(Type type);
};

template <> struct coerce<FatalError::Type> { typedef word type; };

} // namespace neutrino

#endif // _VALUE_VALUE
