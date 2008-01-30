#ifndef _VALUES
#define _VALUES

#include "heap/ref.h"
#include "utils/vector.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/string.h"

namespace neutrino {

enum InstanceType {
  __first_instance_type = 0
#define DECLARE_INSTANCE_TYPE(n, NAME, Name, info) , NAME##_TYPE = n
FOR_EACH_DECLARED_TYPE(DECLARE_INSTANCE_TYPE)
#undef DECLARE_INSTANCE_TYPE
};

#define DECLARE_FIELD(Type, name)                                    \
  inline Type &name();                                               \
  inline void set_##name(Type value)


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
  
  enum WriteMode { DEFAULT, UNQUOTED };
  
  IF_DEBUG(inline InstanceType gc_safe_type());
  IF_DEBUG(Mirror &mirror());
  
  void write_on(string_buffer &buf, WriteMode mode = DEFAULT);
  void write_short_on(string_buffer &buf, WriteMode mode = DEFAULT);
  
  string to_string(WriteMode mode = DEFAULT);
  string to_short_string();
  
};


// -----------------
// --- V a l u e ---
// -----------------

class Value : public Data {
public:
  
  inline InstanceType type();
  
  /**
   * Returns true if this object supports comparisons with other
   * objecs.
   */
  bool is_key();

  /**
   * Checks that this object is consistent; that all fields are legal
   * and hold objects of the expected type.  It always returns false,
   * since it fails by aborting execution, but has a boolean return
   * type to allow it to be used in ASSERTs.
   */
  IF_DEBUG(bool validate());
  
  /**
   * Built-in support for comparing value objects.  This method
   * aborts if the receiver or the argument does not support value
   * comparison, that is, structural rather than identity comparison.
   */
  bool equals(Value *that);
  
  /**
   * Identity comparison.  This is the same as value comparison for
   * objects that support it and otherwise simple object identity.
   */
  bool is_identical(Value *that);

};

template <> class ref_traits<Value> {
public:
  inline string to_string();
  inline InstanceType type();
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


// -------------------
// --- O b j e c t ---
// -------------------

class FieldVisitor {
public:
  virtual void visit_field(Value **field) = 0;
};

#define DECLARE_OBJECT_FIELD(Type, name, Name, arg) DECLARE_FIELD(Type*, name);
#define DECLARE_REF_FIELD(Type, name, Name, arg) inline ref<Type> name();

class Object : public Value {
public:
  DECLARE_FIELD(Layout*, layout);
  IF_DEBUG(inline Layout *gc_safe_layout());
  
  /**
   * The header of an object is the first field which normally
   * contains the class, but may during garbage collection contain a
   * forward pointer to the migrated version of this object.  It
   * should not be used outside of garbage collection.
   */
  DECLARE_FIELD(Data*, header);
  
  void for_each_field(FieldVisitor &visitor);
  uint32_t size_in_memory();
  void preprocess();
  void postprocess();
  
  static const int kHeaderOffset = 0;
  static const int kHeaderSize   = kHeaderOffset + kPointerSize;
};

template <> class ref_traits<Object> : public ref_traits<Value> {
public:
  inline ref<Layout> layout();
};

DEFINE_REF_CLASS(Object);


// -----------------
// --- S t a c k ---
// -----------------

struct StackStatus {
  /**
   * This constructor specifies the state of the stack flags when
   * a stack is first created
   */
  StackStatus()
      : is_empty(true)
#ifdef DEBUG
      , is_cooked(true)
      , is_parked(false)
#endif
      { }

  bool is_empty : 1;
  IF_DEBUG(bool is_cooked : 1);
  IF_DEBUG(bool is_parked : 1);
};

class Stack : public Object {
public:

  /**
   * Make StackFlags seem like a nested class, Stack::Flags.  The
   * reason it is not a true nested class is that it has to be forward
   * declared.
   */
  typedef StackStatus Status;
  
  DECLARE_FIELD(uint32_t, height);
  DECLARE_FIELD(uint32_t, fp);
  DECLARE_FIELD(word*,    top_marker);
  DECLARE_FIELD(Status, status);
  inline word *bottom();
  
  IF_DEBUG(void validate_stack());
  void for_each_stack_field(FieldVisitor &visitor);
  
  /**
   * Converts all derived pointers in this object into neutral
   * integers.
   */
  void uncook_stack();
  
  /**
   * Converts all un-cooked derived pointers back into real ones.
   */
  void recook_stack();

  static inline uint32_t size_for(uint32_t height);
  static const uint32_t kInitialHeight = 2048;
  
  static const uint32_t kHeightOffset    = Object::kHeaderSize;
  static const uint32_t kFpOffset        = kHeightOffset + kPointerSize;
  static const uint32_t kTopMarkerOffset = kFpOffset + kPointerSize;
  static const uint32_t kStatusOffset    = kTopMarkerOffset + kPointerSize;
  static const uint32_t kHeaderSize      = kStatusOffset + kPointerSize;
};

template <> class ref_traits<Stack> : public ref_traits<Object> {
};

DEFINE_REF_CLASS(Stack);


// ---------------
// --- T a s k ---
// ---------------

#define FOR_EACH_TASK_FIELD(VISIT, arg)                              \
  VISIT(Stack, stack, Stack, arg)

class Task : public Object {
public:
  FOR_EACH_TASK_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kStackOffset = Object::kHeaderSize;
  static const uint32_t kSize = kStackOffset + kPointerSize;
};

template <> class ref_traits<Task> : public ref_traits<Object> {
public:
  FOR_EACH_TASK_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Task);


// -----------------------
// --- I n s t a n c e ---
// -----------------------

class Instance : public Object {
public:
  inline Value *&get_field(uint32_t index);
  inline void set_field(uint32_t index, Value *value);
  
  static inline uint32_t size_for(uint32_t elms);
  
  static const uint32_t kHeaderSize = Object::kHeaderSize;
};


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
  
  template <typename T> inline vector<T> buffer();
  
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
  DECLARE_FIELD(uint32_t, length);
  inline char &at(uint32_t index);
  inline void set(uint32_t index, char value);
  
  bool string_equals(String *that);
  bool starts_with_vowel();
  
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
  inline vector<uint16_t> buffer();
  inline uint32_t length();
};

template <>
class ref_traits<Code> : public ref_traits<AbstractBuffer> {
public:
  inline uint16_t &at(uint32_t index);
  inline uint32_t length();
};

DEFINE_REF_CLASS(Code);


// -----------------
// --- T u p l e ---
// -----------------

class Tuple : public Object {
public:
  DECLARE_FIELD(uint32_t, length);
  inline vector<Value*> buffer();
  inline Value *&get(uint32_t index);
  inline void set(uint32_t index, Value *value);
  
  static inline uint32_t size_for(uint32_t elms);
  
  static const int kLengthOffset = Object::kHeaderSize;
  static const int kHeaderSize   = kLengthOffset + kPointerSize;
};

template <> class ref_traits<Tuple> : public ref_traits<Object> {
public:
  inline uint32_t length();
  inline ref<Value> get(uint32_t index);
  inline void set(uint32_t index, ref<Value> value);
};

DEFINE_REF_CLASS(Tuple);


// ---------------------------
// --- D i c t i o n a r y ---
// ---------------------------

#define FOR_EACH_DICTIONARY_FIELD(VISIT, arg)                        \
  VISIT(Tuple, table, Table, arg)

class Dictionary : public Object {
public:
  FOR_EACH_DICTIONARY_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  uint32_t size();
  
  Data *get(Value *key);
  bool set(Value *key, Value *value);
  
  class Iterator {
  public:
    class Entry {
    public:
      Entry() : key(NULL), value(NULL) { }
      Value *key, *value;
    };
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
  FOR_EACH_DICTIONARY_FIELD(DECLARE_REF_FIELD, 0)
  inline ref<Value> get(ref<Value> key);
  inline void set(ref<Value> key, ref<Value> value);
  inline uint32_t size();
};

DEFINE_REF_CLASS(Dictionary);


// -------------------
// --- L a m b d a ---
// -------------------

#define FOR_EACH_LAMBDA_FIELD(VISIT, arg)                            \
  VISIT(Value,   code,          Code,         arg)                   \
  VISIT(Value,   constant_pool, ConstantPool, arg)                   \
  VISIT(Value,   tree,          Tree,         arg)                   \
  VISIT(Tuple,   outers,        Outers,       arg)                   \
  VISIT(Context, context,       Context,      arg)

class Lambda : public Object {
public:
  DECLARE_FIELD(uint32_t, argc);
  FOR_EACH_LAMBDA_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  Value *call(Task *task);
  Data *clone(Heap &heap);
  void ensure_compiled();

  string disassemble();
  
  static const uint32_t kArgcOffset         = Object::kHeaderSize;
  static const uint32_t kCodeOffset         = kArgcOffset + kPointerSize;
  static const uint32_t kConstantPoolOffset = kCodeOffset + kPointerSize;
  static const uint32_t kTreeOffset         = kConstantPoolOffset + kPointerSize;
  static const uint32_t kOutersOffset       = kTreeOffset + kPointerSize;
  static const uint32_t kContextOffset      = kOutersOffset + kPointerSize;
  static const uint32_t kSize               = kContextOffset + kPointerSize;
};

template <> class ref_traits<Lambda> : public ref_traits<Object> {
public:
  FOR_EACH_LAMBDA_FIELD(DECLARE_REF_FIELD, 0)
  void ensure_compiled();
};

DEFINE_REF_CLASS(Lambda);


// -------------------------
// --- S i n g l e t o n ---
// -------------------------

class Singleton : public Object {
public:
  static const int kSize = Object::kHeaderSize;
};

#define FOR_EACH_VOID_FIELD(VISIT, arg)
class Void : public Singleton { };

#define FOR_EACH_NULL_FIELD(VISIT, arg)
class Null : public Singleton { };

#define FOR_EACH_TRUE_FIELD(VISIT, arg)
class True : public Singleton { };

#define FOR_EACH_FALSE_FIELD(VISIT, arg)
class False : public Singleton { };


// -----------------------
// --- P r o t o c o l ---
// -----------------------

#define FOR_EACH_PROTOCOL_FIELD(VISIT, arg)                          \
  VISIT(Value, name,    Name,    arg)                                \
  VISIT(Tuple, methods, Methods, arg)

class Protocol : public Object {
public:
  FOR_EACH_PROTOCOL_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kNameOffset    = Object::kHeaderSize;
  static const uint32_t kMethodsOffset = kNameOffset + kPointerSize;
  static const uint32_t kSize          = kMethodsOffset + kPointerSize;
};

template <> class ref_traits<Protocol> : public ref_traits<Object> {
public:
  FOR_EACH_PROTOCOL_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Protocol);


// -----------------
// --- C l a s s ---
// -----------------

#define FOR_EACH_LAYOUT_FIELD(VISIT, arg)                            \
  VISIT(Tuple, methods, Methods, arg)                                \
  VISIT(Value, super,   Super,   arg)                                \
  VISIT(Value, name,    Name,    arg)

class Layout : public Object {
public:
  DECLARE_FIELD(InstanceType, instance_type);
  DECLARE_FIELD(uint32_t, instance_field_count);
  FOR_EACH_LAYOUT_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  bool is_empty();
  Data *clone(Heap &heap);

  IF_DEBUG(static uint32_t tag_of(Data *value));
  IF_DEBUG(static const char *tag_name(uint32_t tag));
  IF_DEBUG(static const char *layout_name(uint32_t tag));

  static const uint32_t kInstanceTypeOffset       = Object::kHeaderSize;
  static const uint32_t kInstanceFieldCountOffset = kInstanceTypeOffset + kPointerSize;
  static const uint32_t kMethodsOffset            = kInstanceFieldCountOffset + kPointerSize;
  static const uint32_t kSuperOffset              = kMethodsOffset + kPointerSize;
  static const uint32_t kNameOffset               = kSuperOffset + kPointerSize;
  static const uint32_t kSize                     = kNameOffset + kPointerSize;
};

template <> class ref_traits<Layout> {
public:
  FOR_EACH_LAYOUT_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Layout);


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
#define DECLARE_SIGNAL_TYPE(n, NAME, Name, info) , NAME
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

/**
 * A forward pointer is a value used by the garbage collector.  When
 * an object has been moved to new space, a forward pointer value is
 * written into the object's header field and can be used to find the
 * new incarnation of the object.
 *
 * Forward pointers are tagged the same way as signals which can be
 * confusing is you're not careful.  Signals are never stores in the
 * heap and are not used by the garbage collector.  Forward pointers
 * are only used in the heap and are never used outside the garbage
 * collector.
 */
class ForwardPointer : public Data {
 public:
  static inline ForwardPointer *make(Object *target);
  inline Object *target();
};

} // neutrino

#include "values/method.h"

#endif // _VALUES
