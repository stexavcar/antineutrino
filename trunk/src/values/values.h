#ifndef _VALUES
#define _VALUES

#include "heap/ref.h"
#include "utils/vector.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/string.h"

namespace neutrino {

enum InstanceType {
  __illegal_instance_type = -1,
  __first_instance_type = 0
#define DECLARE_INSTANCE_TYPE(n, NAME, Name, info) , t##Name = n
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

  /**
   * Returns true if this object supports comparisons with other
   * objecs.
   */
  bool is_key();
  
  /**
   * Returns true if this object can be compared with other objects,
   * and equality of this object does not involve testing other
   * objects for equality.  For instance, strings are atomic keys
   * whereas tuples are not since the tuple elements must be compared.
   */
  bool is_atomic_key();

};

template <> class ref_traits<Value> {
public:
  inline InstanceType type();
};

DEFINE_REF_CLASS(Value);

// -------------------------
// --- I m m e d i a t e ---
// -------------------------

class Immediate : public Value {
public:

  /**
   * Checks that this object is consistent; that all fields are legal
   * and hold objects of the expected type.  It always returns false,
   * since it fails by aborting execution, but has a boolean return
   * type to allow it to be used in ASSERTs.
   */
  IF_DEBUG(bool validate());

};

template <> class ref_traits<Immediate> : public ref_traits<Value> {
public:
  inline string to_string();
};

DEFINE_REF_CLASS(Immediate);


// ---------------------------------
// --- S m a l l   I n t e g e r ---
// ---------------------------------

class Smi : public Immediate {
public:
  inline word value();

  static inline Smi *from_int(word value);
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

class Object : public Immediate {
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
  uword size_in_memory();
  void preprocess();
  void postprocess();

  static const int kHeaderOffset = 0;
  static const int kHeaderSize   = kHeaderOffset + kPointerSize;
};

template <> class ref_traits<Object> : public ref_traits<Immediate> {
public:
  inline ref<Layout> layout();
};

DEFINE_REF_CLASS(Object);


// -------------------------
// --- F o r w a r d e r ---
// -------------------------

/**
 * A transparent forwarder that delegates all access to a wrapped
 * object.  In reality it is a differently tagged object pointer
 * that either points directly to the delegate object, or to a
 * forward pointer descriptor object that holds a pointer to the
 * delegate object.
 */
class Forwarder : public Value {
public:
  inline ForwarderDescriptor *descriptor();

  enum Type {

    /**
     * An open forwarder is one whose value may change.  It is not
     * safe to replace an open forwarder with its value.
     */
    fwOpen,
    
    /**
     * A closed forwarder is one whose value is fixed.  It is safe
     * to replace it with its value, and indeed the garbage collector
     * should eventually replace all instances of the forwarder with
     * their values.
     */
    fwClosed

  };

  static inline Forwarder *to(ForwarderDescriptor *obj);
};

#define FOR_EACH_FORWARDER_DESCRIPTOR_FIELD(VISIT, arg)              \
  VISIT(Value, target, Target, arg)

class ForwarderDescriptor : public Object {
public:
  inline Forwarder::Type &type();
  inline void set_type(Forwarder::Type value);
  inline void set_raw_type(Forwarder::Type value);

  FOR_EACH_FORWARDER_DESCRIPTOR_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kTypeOffset   = Object::kHeaderSize;
  static const uword kTargetOffset = kTypeOffset + kPointerSize;
  static const uword kSize         = kTargetOffset + kPointerSize;
};

template <> class ref_traits<ForwarderDescriptor> : public ref_traits<Object> {
public:
  FOR_EACH_FORWARDER_DESCRIPTOR_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(ForwarderDescriptor);


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

  DECLARE_FIELD(uword, height);
  DECLARE_FIELD(uword, fp);
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

  static inline uword size_for(uword height);
  static const uword kInitialHeight = 2048;

  static const uword kHeightOffset    = Object::kHeaderSize;
  static const uword kFpOffset        = kHeightOffset + kPointerSize;
  static const uword kTopMarkerOffset = kFpOffset + kPointerSize;
  static const uword kStatusOffset    = kTopMarkerOffset + kPointerSize;
  static const uword kHeaderSize      = kStatusOffset + kPointerSize;
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

  static const uword kStackOffset = Object::kHeaderSize;
  static const uword kSize = kStackOffset + kPointerSize;
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
  inline Value *&get_field(uword index);
  inline void set_field(uword index, Value *value);

  void for_each_instance_field(FieldVisitor &visitor);
  static inline uword size_for(uword elms);

  static const uword kHeaderSize = Object::kHeaderSize;
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
  template <typename T> inline uword size();

  /**
   * Sets the size of this buffer when viewed as the given type.
   * There is no reason to use this function outside of low-level
   * heap management.
   */
  template <typename T> inline void set_size(uword);

  /**
   * Returns the element at the specified index, viewing the buffer as
   * an array of the specified element type.  The index is an element
   * and not a byte index, so you will read from the same memory when
   * reading the 50th element of a buffer of int32s as when you read
   * the 200th element of the same buffer when read as an array of
   * int8s.
   */
  template <typename T> inline T &at(uword index);

  template <typename T> inline vector<T> buffer();

  static inline uword size_for(uword byte_count);

  static const int kSizeOffset = Object::kHeaderSize;
  static const int kHeaderSize = kSizeOffset + kPointerSize;
};

template <>
class ref_traits<AbstractBuffer> : public ref_traits<Object> {
public:
  template <typename T> inline uword &size();
  template <typename T> inline T &at(uword index);
};

DEFINE_REF_CLASS(AbstractBuffer);


// -------------------
// --- S t r i n g ---
// -------------------

class String : public Object {
public:
  DECLARE_FIELD(uword, length);
  inline char &at(uword index);
  inline void set(uword index, char value);

  bool string_equals(String *that);
  bool starts_with_vowel();
  vector<char> c_str();

  static inline uword size_for(uword chars);

  static const int kLengthOffset = Object::kHeaderSize;
  static const int kHeaderSize   = kLengthOffset + kPointerSize;
};

template <>
class ref_traits<String> : public ref_traits<AbstractBuffer> {
public:
  inline vector<char> c_str();
};

DEFINE_REF_CLASS(String);


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
  inline uint16_t &at(uword index);
  inline vector<uint16_t> buffer();
  inline uword length();
};

template <>
class ref_traits<Code> : public ref_traits<AbstractBuffer> {
public:
  inline uint16_t &at(uword index);
  inline uword length();
};

DEFINE_REF_CLASS(Code);


// -----------------------------------
// --- A b s t r a c t   T u p l e ---
// -----------------------------------

class AbstractTuple : public Object {
public:
  DECLARE_FIELD(uword, length);
  inline vector<Value*> buffer();
  inline Value *&get(uword index);
  inline void set(uword index, Value *value);
  
  inline bool is_empty();
  bool tuple_equals(Tuple *that);

  static inline uword size_for(uword elms);

  static const int kLengthOffset = Object::kHeaderSize;
  static const int kHeaderSize   = kLengthOffset + kPointerSize;
};

template <> class ref_traits<AbstractTuple> : public ref_traits<Object> {
public:
  inline uword length();
  inline ref<Value> get(uword index);
  inline void set(uword index, ref<Value> value);
};

DEFINE_REF_CLASS(AbstractTuple);


// -----------------
// --- T u p l e ---
// -----------------

class Tuple : public AbstractTuple { };

template <> class ref_traits<Tuple> : public ref_traits<AbstractTuple> {
public:
};

DEFINE_REF_CLASS(Tuple);


// -----------------
// --- A r r a y ---
// -----------------

/**
 * An array is exactly the same as a tuple except that it has a
 * different layout which allows code to write entries.
 */
class Array : public AbstractTuple { };

template <> class ref_traits<Array> : public ref_traits<AbstractTuple> {
public:
};

DEFINE_REF_CLASS(Array);


// ---------------------------
// --- D i c t i o n a r y ---
// ---------------------------

#define FOR_EACH_DICTIONARY_FIELD(VISIT, arg)                        \
  VISIT(Tuple, table, Table, arg)

class Dictionary : public Object {
public:
  FOR_EACH_DICTIONARY_FIELD(DECLARE_OBJECT_FIELD, 0)

  uword size();

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
    uword index() { return index_; }
    Tuple *table_;
    uword index_;
  };

  static const int kTableOffset = Object::kHeaderSize;
  static const int kSize        = kTableOffset + kPointerSize;
};

template <> class ref_traits<Dictionary> : public ref_traits<Object> {
public:
  FOR_EACH_DICTIONARY_FIELD(DECLARE_REF_FIELD, 0)
  inline ref<Value> get(ref<Value> key);
  inline void set(ref<Value> key, ref<Value> value);
  inline uword size();
};

DEFINE_REF_CLASS(Dictionary);


// -------------------
// --- L a m b d a ---
// -------------------

#define FOR_EACH_LAMBDA_FIELD(VISIT, arg)                            \
  VISIT(Value,      code,          Code,         arg)                \
  VISIT(Value,      constant_pool, ConstantPool, arg)                \
  VISIT(Value,      tree,          Tree,         arg)                \
  VISIT(Tuple,      outers,        Outers,       arg)                \
  VISIT(Context,    context,       Context,      arg)

class Lambda : public Object {
public:
  DECLARE_FIELD(uword, argc);
  FOR_EACH_LAMBDA_FIELD(DECLARE_OBJECT_FIELD, 0)

  Value *call(Task *task);
  Data *clone(Heap &heap);
  void ensure_compiled(Method *holder);

  string disassemble();

  static const uword kArgcOffset         = Object::kHeaderSize;
  static const uword kCodeOffset         = kArgcOffset + kPointerSize;
  static const uword kConstantPoolOffset = kCodeOffset + kPointerSize;
  static const uword kTreeOffset         = kConstantPoolOffset + kPointerSize;
  static const uword kOutersOffset       = kTreeOffset + kPointerSize;
  static const uword kContextOffset      = kOutersOffset + kPointerSize;
  static const uword kSize               = kContextOffset + kPointerSize;
};

template <> class ref_traits<Lambda> : public ref_traits<Object> {
public:
  FOR_EACH_LAMBDA_FIELD(DECLARE_REF_FIELD, 0)
  void ensure_compiled(ref<Method> holder);
};

DEFINE_REF_CLASS(Lambda);


// -------------------------
// --- S i n g l e t o n ---
// -------------------------

class Singleton : public Object {
public:
  static const uword kSize = Object::kHeaderSize;
};

class Bool : public Singleton {
public:
  static const uword kSize = Singleton::kSize;
};

#define FOR_EACH_VOID_FIELD(VISIT, arg)
class Void : public Singleton { };

#define FOR_EACH_NULL_FIELD(VISIT, arg)
class Null : public Singleton { };

#define FOR_EACH_TRUE_FIELD(VISIT, arg)
class True : public Bool { };

#define FOR_EACH_FALSE_FIELD(VISIT, arg)
class False : public Bool { };


// -----------------------
// --- P r o t o c o l ---
// -----------------------

#define FOR_EACH_PROTOCOL_FIELD(VISIT, arg)                          \
  VISIT(Immediate, name,    Name,    arg)                            \
  VISIT(Tuple,     methods, Methods, arg)                            \
  VISIT(Value,     super,   Super,   arg)

class Protocol : public Object {
public:
  FOR_EACH_PROTOCOL_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset    = Object::kHeaderSize;
  static const uword kMethodsOffset = kNameOffset + kPointerSize;
  static const uword kSuperOffset   = kMethodsOffset + kPointerSize;
  static const uword kSize          = kSuperOffset + kPointerSize;
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
  VISIT(Immediate, protocol, Protocol, arg)                          \
  VISIT(Tuple,     methods,  Methods,  arg)

class Layout : public Object {
public:
  DECLARE_FIELD(InstanceType, instance_type);
  DECLARE_FIELD(uword, instance_field_count);
  FOR_EACH_LAYOUT_FIELD(DECLARE_OBJECT_FIELD, 0)

  bool is_empty();
  Data *clone(Heap &heap);

  IF_DEBUG(static uword tag_of(Data *value));
  IF_DEBUG(static const char *tag_name(uword tag));

  static const char *layout_name(uword tag);
  static const uword kInstanceTypeOffset       = Object::kHeaderSize;
  static const uword kInstanceFieldCountOffset = kInstanceTypeOffset + kPointerSize;
  static const uword kProtocolOffset           = kInstanceFieldCountOffset + kPointerSize;
  static const uword kMethodsOffset            = kProtocolOffset + kPointerSize;
  static const uword kSize                     = kMethodsOffset + kPointerSize;
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
  inline uword type();
  inline uword payload();
  enum Type {
    FIRST_SIGNAL_TYPE = -1
#define DECLARE_SIGNAL_TYPE(n, NAME, Name, info) , s##Name
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

#include "values/channel.h"
#include "values/method.h"

#endif // _VALUES
