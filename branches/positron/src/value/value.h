#ifndef _VALUE_VALUE
#define _VALUE_VALUE

#include "utils/array.h"
#include "utils/global.h"
#include "utils/string.h"
#include "value/pointer.h"

namespace neutrino {

#define eConcreteValueTypes(VISIT)                                   \
  VISIT(Species)                                                     \
  eSimpleConcreteValueTypes(VISIT)

#define eSimpleConcreteValueTypes(VISIT)                                \
  VISIT(Blob)       VISIT(String)     VISIT(Array)      VISIT(Instance) \
  VISIT(SyntaxTree) VISIT(Nil)        VISIT(HashMap)

#define eSignalTypes(VISIT)                                             \
  VISIT(FatalError) VISIT(InternalError) VISIT(Success)

class Data {
public:
  enum DataType { dtSignal, dtUnknown, __last_data_type };
  DataType type();
  void println();
};

inline void encode_variant(variant &that, Data *value);

template <class C>
static inline bool is(Data *data);

template <class C>
static inline C *cast(Data *data);

class Value : public Data {
public:
  enum Type {
    __first = Data::__last_data_type
#define MAKE_ENUM(Name) , t##Name
    eConcreteValueTypes(MAKE_ENUM)
#undef MAKE_ENUM
  };

  word hash();

  static Value *nothing() { return NULL; }
  bool is_nothing() { return this == nothing(); }

};

class TaggedInteger : public Value {
public:
  inline word value();
  static inline bool fits(word value);
  static inline TaggedInteger *make(word value);

  static const word kUpperLimit = Pointer::kTaggedIntegerUpperLimit;
  static const word kLowerLimit = Pointer::kTaggedIntegerLowerLimit;
};

/* --- S m a l l   D o u b l e --- */

class SmallDouble : public Value {
public:
  inline double value();
};

/* --- O b j e c t --- */

class Object : public Value {
public:
  inline explicit Object(Species *species);
  inline Type type();
  inline Species *species();
  inline void set_species(Species *v);
  inline void set_forwarding_header(ForwardPointer *v);

  void migrate_fields(FieldMigrator &migrator);
  word calculate_hash();

  template <typename T>
  static allocation<Object> clone_object(Species *desc, Object *obj,
      Space &space);

  template <typename T>
  static word object_size_in_memory(Species *desc, Object *obj);

  template <typename T>
  static void migrate_object_fields(Species *desc, Object *obj,
      FieldMigrator &migrator);

  template <typename T>
  static word object_hash(Object *obj);

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
    // Calculate the hash code of an object
    word (*hash)(Object*);
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
  static allocation<Object> clone_object(Species *desc, Object *obj,
      Space &space);

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

/* --- A r r a y --- */

class Array : public Object {
public:
  Array(Species *species, word length)
    : Object(species)
    , length_(length) { }

  inline array<Value*> elements();
  inline vector<Value*> as_vector();
  inline void set(word index, Value *value);
  inline Value *get(word index);
  inline word length() { return length_; }

  void migrate_fields(FieldMigrator &migrator);
  static inline word size_in_memory(word elements);

  template <typename T>
  static allocation<Object> clone_object(Species *desc, Object *obj,
      Space &space);

  template <typename T>
  static word object_size_in_memory(Species *desc, Object *obj);

  static Virtuals &virtuals() { return kVirtuals; }

private:
  word length_;
  static Virtuals kVirtuals;
};

/* --- H a s h   M a p --- */

class HashMap : public Object {
public:
  HashMap(Species *species, Array *table)
    : Object(species)
    , table_(table) { }

  class Entry {
  public:
    Entry(HashMap *map, word index) : map_(map), index_(index) { }
    bool is_occupied();
    Value *&key();
    Value *&value();
    Value *&hash();
    static const word kSize = 3;
  private:
    HashMap *map() { return map_; }
    word index() { return index_; }
    array<Value*> entries();
    static const word kKeyOffset = 0;
    static const word kValueOffset = 1;
    static const word kHashOffset = 2;
    HashMap *map_;
    word index_;
  };

  possibly set(Value *key, Value *value);
  Data *get(Value *key);

  void migrate_fields(FieldMigrator &migrator);

  static Virtuals &virtuals() { return kVirtuals; }
  static const word kInitialCapacity = 8;

private:
  Entry lookup(TaggedInteger *hash, Value *key);
  Array *table() { return table_; }

  word size_;
  Array *table_;
  static Virtuals kVirtuals;
};

/* --- N u l l --- */

class Nil : public Object {
public:
  Nil(Species *species) : Object(species) { }
  static Virtuals &virtuals() { return kVirtuals; }
private:
  static Virtuals kVirtuals;
};

/* --- I n s t a n c e --- */

class Instance : public Object {
public:
  static inline word size_in_memory(word fields);
  static Virtuals &virtuals() { return kVirtuals; }
private:
  static Virtuals kVirtuals;
};

class InstanceSpecies : public Species {
public:
  InstanceSpecies(Species *meta, word field_count)
    : Species(meta, tInstance, Instance::virtuals())
    , field_count_(field_count) { }
  word field_count() { return field_count_; }

private:
  word field_count_;
};

/* --- B l o b --- */

class Blob : public Object {
public:
  Blob(Species *species, word length)
    : Object(species)
    , length_(length) { }

  template <typename T> word length() { return length_ / sizeof(T); }

  template <typename T> array<T> data();
  template <typename T> vector<T> as_vector();

  template <typename T>
  static allocation<Object> clone_object(Species *desc, Object *obj,
      Space &space);

  template <typename T>
  static word object_size_in_memory(Species *desc, Object *obj);

  static inline word size_in_memory(word length);

  static Virtuals &virtuals() { return kVirtuals; }
private:
  word length_;
  static Virtuals kVirtuals;
};

/* --- A s t --- */

class SyntaxTree : public Object {
public:
  SyntaxTree(Species *species, Blob *code, Array *literals)
    : Object(species)
    , code_(code)
    , literals_(literals) { }
  void migrate_fields(FieldMigrator &migrator);
  bool disassemble(string_stream &out);
  static Virtuals &virtuals() { return kVirtuals; }
  Blob *code() { return code_; }
  Array *literals() { return literals_; }
private:
  Blob *code_;
  Array *literals_;
  static Virtuals kVirtuals;
};

/* --- S t r i n g --- */

class String : public Object {
public:
  String(Species *species, word length)
    : Object(species)
    , length_(length) { }

  inline array<code_point> chars();
  inline vector<code_point> as_vector();
  word length() { return length_; }
  bool equals(const string &str);

  word calculate_hash();

  template <typename T>
  static allocation<Object> clone_object(Species *desc, Object *obj,
      Space &space);

  template <typename T>
  static word object_size_in_memory(Species *desc, Object *obj);

  static inline word size_in_memory(word chars);

  static Virtuals &virtuals() { return kVirtuals; }

private:
  word length_;
  static Virtuals kVirtuals;
};

/* --- S i g n a l --- */

class ForwardPointer : public Data {
public:
  inline Object *target();
  static inline ForwardPointer *make(Object *value);
};

class Signal : public Data {
public:
  enum Type {
    __first
#define MAKE_ENUM(Name) , s##Name
    eSignalTypes(MAKE_ENUM)
#undef MAKE_ENUM
  };
  inline Type type();
  inline word payload();
};

class Success : public Signal {
public:
  void print_on(string modifiers, string_stream &out);
  static inline Success *make();
};

class Failure : public Signal {
};

#define eInternalErrorTypes(VISIT)                                   \
  VISIT(Unknown, unknown) VISIT(HeapExhaustion, heap_exhaustion)     \
  VISIT(System, system)   VISIT(Environment, environment)

class InternalError : public Failure {
public:
  enum Type {
    __first
#define MAKE_ENUM(Name, name) , ie##Name
    eInternalErrorTypes(MAKE_ENUM)
#undef MAKE_ENUM
  };
  void print_on(string modifiers, string_stream &out);
#define MAKE_CONSTRUCTOR(Name, name) static inline InternalError *name();
  eInternalErrorTypes(MAKE_CONSTRUCTOR)
#undef MAKE_CONSTRUCTOR
private:
  static inline InternalError *make(Type type);
};

#define eFatalErrorTypes(VISIT)                                      \
  VISIT(OutOfMemory, out_of_memory)                                  \
  VISIT(Abort, abort)

class FatalError : public Failure {
public:
  enum Error {
    __first
#define MAKE_ENUM(Name, name) , fe##Name
    eFatalErrorTypes(MAKE_ENUM)
#undef MAKE_ENUM
  };
  void print_on(string modifiers, string_stream &out);
  inline Error error();
  static inline FatalError *out_of_memory();
  static inline FatalError *abort();
private:
  static inline FatalError *make(Error type);
};

} // namespace neutrino

#endif // _VALUE_VALUE
