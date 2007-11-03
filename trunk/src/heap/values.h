#ifndef _VALUES
#define _VALUES

#include "heap/ref.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/string.h"

namespace neutrino {


enum InstanceType {
  ILLEGAL_TAG = 0
#define DECLARE_INSTANCE_TYPE(n, NAME, Name) , NAME##_TYPE = n
FOR_EACH_DECLARED_TYPE(DECLARE_INSTANCE_TYPE)
#undef DECLARE_INSTANCE_TYPE
};


// ---------------
// --- D a t a ---
// ---------------

/**
 * The superclass of all object/value/signal references in neutrino.
 * A data pointer can either be a proper neutrino object or an
 * internal signal.
 */
class Data {
public:
  
  void write_on(string_buffer &buf);
  void write_short_on(string_buffer &buf);
  
  void print(FILE *out = stdout);
  string to_string();
  string to_short_string();
  
};


// -----------------
// --- V a l u e ---
// -----------------

class Value : public Data {
public:
  
  /**
   * Returns true if this object supports comparisons with other
   * objecs.
   */
  IF_DEBUG(bool is_key();)
  
  /**
   * Built-in support for comparing certain objects.  This method
   * aborts if the receiver or the argument does not support object
   * comparison.
   */
  bool equals(Value *that);

};

template <> class ref_traits<Value> {
public:
  inline string to_string();
  inline void print(FILE *out = stdout);
};

DEFINE_REF_CLASS(Value);


// ---------------------------------
// --- S m a l l   I n t e g e r ---
// ---------------------------------

class Smi : public Value {
public:
  inline int32_t value();

  static inline Smi *from_int(int32_t value);
};

class Object : public Value {
public:
  inline Class *chlass();
  inline void set_chlass(Class*);
  
  static const int kChlassOffset = 0;
  static const int kHeaderSize   = kChlassOffset + kPointerSize;
};

template <> class ref_traits<Object> : public ref_traits<Value> { };


// -------------------------------------
// --- A b s t r a c t   B u f f e r ---
// -------------------------------------

/**
 * The superclass of objects that consist of a length field and then
 * a length-sized buffer of raw data.
 */
class AbstractBuffer : public Object {
public:
  
  /**
   * Returns the size of this buffer when the elements are viewed as
   * the specified type.  For instance, if this method return 100 when
   * viewed as an array of int32s it will return 400 when viewed as
   * an array of int8s.
   */
  template <typename T> inline uint32_t size();
  
  /**
   * Sets the size of this buffer when viewed as the given type.
   * There is no reason to use this function outside of low-level
   * heap management.
   */
  template <typename T> inline void set_size(uint32_t);
  
  /**
   * Returns the element at the specified index, viewing the buffer as
   * an array of the specified element type.  The index is an element
   * and not a byte index, so you will read from the same memory when
   * reading the 50th element of a buffer of int32s as when you read
   * the 200th element of the same buffer when read as an array of
   * int8s.
   */
  template <typename T> inline T &at(uint32_t index);
  
  static inline uint32_t size_for(uint32_t byte_count);
  
  static const int kSizeOffset = Object::kHeaderSize;
  static const int kHeaderSize = kSizeOffset + kPointerSize;
};

template <>
class ref_traits<AbstractBuffer> : public ref_traits<Object> {
public:
  template <typename T> inline uint32_t &size();
  template <typename T> inline T &at(uint32_t index);
};

DEFINE_REF_CLASS(AbstractBuffer);


// -------------------
// --- S t r i n g ---
// -------------------

class String : public Object {
public:
  inline uint32_t &length();
  inline char &at(uint32_t index);
  
  bool string_equals(String *that);
  
  static inline uint32_t size_for(uint32_t chars);

  static const int kLengthOffset = Object::kHeaderSize;
  static const int kHeaderSize   = kLengthOffset + kPointerSize;
};


// -------------------
// --- B u f f e r ---
// -------------------

/**
 * A fixed-length piece of memory which can be used for... whatever.
 */
class Buffer : public AbstractBuffer {
  
};


// ---------------
// --- C o d e ---
// ---------------

class Code : public AbstractBuffer {
public:
  inline uint16_t &at(uint32_t index);
};

template <>
class ref_traits<Code> : public ref_traits<AbstractBuffer> {
public:
  inline uint16_t &at(uint32_t index);
};

DEFINE_REF_CLASS(Code);


// -----------------
// --- T u p l e ---
// -----------------

class Tuple : public Object {
public:
  inline uint32_t &length();
  inline Value *&at(uint32_t index);
  
  static inline uint32_t size_for(uint32_t elms);
  
  static const int kLengthOffset = Object::kHeaderSize;
  static const int kHeaderSize   = kLengthOffset + kPointerSize;
};

template <> class ref_traits<Tuple> : public ref_traits<Object> {
public:
  inline ref<Value> get(uint32_t index);
  inline void set(uint32_t index, ref<Value> value);
};

DEFINE_REF_CLASS(Tuple);


// ---------------------------
// --- D i c t i o n a r y ---
// ---------------------------

class Dictionary : public Object {
public:
  inline Tuple *table();
  inline void set_table(Tuple *value);
  
  Data *get(Value *key);
  bool set(Value *key, Value *value);
  
  class Iterator {
  public:
    struct Entry { Value *key, *value; };
    inline Iterator(Dictionary *dict);
    inline bool next(Entry *entry);
  private:
    Tuple *table() { return table_; }
    uint32_t index() { return index_; }
    Tuple *table_;
    uint32_t index_;
  };
  
  static const int kTableOffset = Object::kHeaderSize;
  static const int kSize        = kTableOffset + kPointerSize;
};

template <> class ref_traits<Dictionary> : public ref_traits<Object> {
public:
  inline ref<Value> get(ref<Value> key);
  inline void set(ref<Value> key, ref<Value> value);  
};

DEFINE_REF_CLASS(Dictionary);


// -------------------
// --- L a m b d a ---
// -------------------

class Lambda : public Object {
public:
  inline uint32_t &argc();
  
  inline Code *code();
  inline void set_code(Code *code);
  
  inline Tuple *literals();
  inline void set_literals(Tuple *literals);
  
  static const int kArgcOffset     = Object::kHeaderSize;
  static const int kCodeOffset     = kArgcOffset + kPointerSize;
  static const int kLiteralsOffset = kCodeOffset + kPointerSize;
  static const int kSize           = kLiteralsOffset + kPointerSize;
};

template <> class ref_traits<Lambda> : public ref_traits<Object> {
public:
  inline ref<Code> code();
  inline ref<Tuple> literals();
  ref<Value> call();
};

DEFINE_REF_CLASS(Lambda);


// -------------------------
// --- S i n g l e t o n ---
// -------------------------

class Singleton : public Object {
public:
  static const int kSize = Object::kHeaderSize;
};

class Void : public Singleton { };
class Null : public Singleton { };
class True : public Singleton { };
class False : public Singleton { };


// -------------------
// --- M e t h o d ---
// -------------------

class Method : public Object {
public:
  inline String *name();
  inline void set_name(String *name);
  
  inline Lambda *lambda();
  inline void set_lambda(Lambda *lambda);
  
  static const int kNameOffset   = Object::kHeaderSize;
  static const int kLambdaOffset = kNameOffset + kPointerSize;
  static const int kSize         = kLambdaOffset + kPointerSize;
};


// -----------------------
// --- P r o t o c o l ---
// -----------------------

class Protocol : public Object {
public:
  inline uint32_t &id();
  
  static const int kIdOffset = Object::kHeaderSize;
  static const int kSize     = kIdOffset + kPointerSize;
};


// -----------------------------
// --- S y n t a x   T r e e ---
// -----------------------------

class SyntaxTree : public Object {
public:
  inline Value *&at(uint32_t index);
  
  static const int kHeaderSize = Object::kHeaderSize;
};


// -----------------
// --- C l a s s ---
// -----------------

class Class : public Object {
public:
  inline uint32_t &instance_type();

  inline Tuple *methods();
  inline void set_methods(Tuple *methods);

  IF_DEBUG(static uint32_t tag_of(Data *value);)
  IF_DEBUG(static const char *tag_name(uint32_t tag);)
  IF_DEBUG(static const char *class_name(uint32_t tag);)

  static const int kInstanceTypeOffset = Object::kHeaderSize;
  static const int kMethodsOffset      = kInstanceTypeOffset + kPointerSize;
  static const int kSize               = kMethodsOffset + kPointerSize;
};


// ---------------------
// --- S i g n a l ---
// ---------------------

/**
 * A value distinct from "real" neutrino values which can be used to
 * carry information internally in the vm.
 */
class Signal : public Data {
public:
  inline uint32_t type();
  inline uint32_t payload();
  enum Type {
    FIRST_SIGNAL_TYPE = -1
#define DECLARE_SIGNAL_TYPE(n, NAME, Name) , NAME
FOR_EACH_SIGNAL_TYPE(DECLARE_SIGNAL_TYPE)
#undef DECLARE_SIGNAL_TYPE
  };
};

class AllocationFailed : public Signal {
public:
  static inline AllocationFailed *make(int size);
};

class InternalError : public Signal {
public:
  static inline InternalError *make(int code);
};

class Nothing : public Signal {
public:
  static inline Nothing *make();
};

class PendingRegister : public Signal {
public:
  static inline PendingRegister *make(uint32_t reg);
};

}

#endif // _VALUES
