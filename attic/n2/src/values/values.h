#ifndef _VALUES
#define _VALUES

#include "heap/ref.h"
#include "utils/array.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/smart-ptrs.h"
#include "utils/string.h"

namespace neutrino {

enum InstanceType {
  __illegal_instance_type = -1,
  __first_instance_type = 0
#define DECLARE_INSTANCE_TYPE(n, Name, info) , t##Name = n
eDeclaredTypes(DECLARE_INSTANCE_TYPE)
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
#define DECLARE_REF_FIELD(Type, name, Name, arg) inline Type *name();

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
  inline Layout *layout();
};

DEFINE_REF_CLASS(Object);


// ---------------
// --- C e l l ---
// ---------------


#define eCellFields(VISIT, arg)                                      \
  VISIT(Value, value, Value, arg)


class Cell : public Object {
public:
  eCellFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kValueOffset = Object::kHeaderSize;
  static const uword kSize        = kValueOffset + kPointerSize;
};


template <> class ref_traits<Cell> : public ref_traits<Object> {
public:
  eCellFields(DECLARE_REF_FIELD, 0)
};


DEFINE_REF_CLASS(Cell);


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
     * protect to replace an open forwarder with its value.
     */
    fwOpen,

    /**
     * A closed forwarder is one whose value is fixed.  It is protect
     * to replace it with its value, and indeed the garbage collector
     * should eventually replace all instances of the forwarder with
     * their values.
     */
    fwClosed

  };

  static inline Forwarder *to(ForwarderDescriptor *obj);
};

#define eForwarderDescriptorFields(VISIT, arg)                       \
  VISIT(Value, target, Target, arg)

class ForwarderDescriptor : public Object {
public:
  inline Forwarder::Type &type();
  inline void set_type(Forwarder::Type value);
  inline void set_raw_type(Forwarder::Type value);

  eForwarderDescriptorFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kTypeOffset   = Object::kHeaderSize;
  static const uword kTargetOffset = kTypeOffset + kPointerSize;
  static const uword kSize         = kTargetOffset + kPointerSize;
};

template <> class ref_traits<ForwarderDescriptor> : public ref_traits<Object> {
public:
  eForwarderDescriptorFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(ForwarderDescriptor);


// -----------------
// --- S t a c k ---
// -----------------

struct StackStatus {
  enum State {
    ssUninitialized, ssParked, ssRunning
  };
  /**
   * This constructor specifies the state of the stack flags when
   * a stack is first created
   */
  StackStatus()
      : is_empty(true)
#ifdef DEBUG
      , is_cooked(true)
      , state(ssUninitialized)
#endif
      { }

  bool is_empty : 1;
  IF_DEBUG(bool is_cooked : 1);
  IF_DEBUG(uint8_t state : 3);
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

  DECLARE_FIELD(word*, fp);
  DECLARE_FIELD(word*, sp);
  DECLARE_FIELD(uword, pc);
  // An alternative model to having all the execution state stored
  // in fields is to just have the fp and then push an activation,
  // since all new activation store the sp and pc.  The reason for
  // not doing that is that when suspending the stack on stack
  // overflow we need room to store these, and reserving room for
  // one activation would waste more space than these extra fields.

  DECLARE_FIELD(word*, top_marker);
  DECLARE_FIELD(Status, status);
  inline word* bottom();
  inline bounded_ptr<word> bound(word *ptr);
  inline array<word> buffer();

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
  static const uword kInitialHeight = 144;

  static const uword kHeightOffset    = Object::kHeaderSize;
  static const uword kFpOffset        = kHeightOffset + kPointerSize;
  static const uword kSpOffset        = kFpOffset + kPointerSize;
  static const uword kPcOffset        = kSpOffset + kPointerSize;
  static const uword kTopMarkerOffset = kPcOffset + kPointerSize;
  static const uword kStatusOffset    = kTopMarkerOffset + kPointerSize;
  static const uword kHeaderSize      = kStatusOffset + kPointerSize;
};

template <> class ref_traits<Stack> : public ref_traits<Object> {
};

DEFINE_REF_CLASS(Stack);


// ---------------
// --- T a s k ---
// ---------------

#define eTaskFields(VISIT, arg)                                      \
  VISIT(Stack,     stack,  Stack,  arg)                              \
  VISIT(Immediate, caller, Caller, arg)

class Task : public Object {
public:
  eTaskFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kStackOffset  = Object::kHeaderSize;
  static const uword kCallerOffset = kStackOffset + kPointerSize;
  static const uword kSize         = kCallerOffset + kPointerSize;
};

template <> class ref_traits<Task> : public ref_traits<Object> {
public:
  eTaskFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Task);


// -----------------------
// --- I n s t a n c e ---
// -----------------------

class Instance : public Object {
public:
  IF_DEBUG(inline InstanceLayout *gc_safe_layout());

  inline Value *&get_field(uword index);
  inline void set_field(uword index, Value *value);

  inline InstanceLayout *layout();

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

  template <typename T> inline array<T> buffer();

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
  inline char &get(uword index);
  inline void set(uword index, char value);

  bool string_equals(String *that);
  bool starts_with_vowel();
  array<char> c_str();

  static inline uword size_for(uword chars);

  static const int kLengthOffset = Object::kHeaderSize;
  static const int kHeaderSize   = kLengthOffset + kPointerSize;
};

template <>
class ref_traits<String> : public ref_traits<AbstractBuffer> {
public:
  inline array<char> c_str();
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
  inline uint16_t get(uword index);
  inline void set(uword index, uint16_t value);
  inline array<uint16_t> buffer();
  inline uword length();
};

template <>
class ref_traits<Code> : public ref_traits<AbstractBuffer> {
public:
  inline uint16_t get(uword index);
  inline void set(uword index, uint16_t value);
  inline uword length();
};

DEFINE_REF_CLASS(Code);


// -----------------------------------
// --- A b s t r a c t   T u p l e ---
// -----------------------------------

class AbstractTuple : public Object {
public:
  DECLARE_FIELD(uword, length);
  inline array<Value*> buffer();
  inline Value *&get(uword index);
  inline void set(uword index, Value *value);

  inline bool is_empty();
  bool tuple_equals(Tuple *that);

  static inline uword size_for(uword elms);

  static const int kLengthOffset = Object::kHeaderSize;
  static const int kHeaderSize   = kLengthOffset + kPointerSize;
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


// -----------------------
// --- H a s h   M a p ---
// -----------------------

#define eHashMapFields(VISIT, arg)                        \
  VISIT(Tuple, table, Table, arg)

class HashMap : public Object {
public:
  eHashMapFields(DECLARE_OBJECT_FIELD, 0)

  uword size();

  Data *get(Value *key);
  Data *set(Heap &heap, Value *key, Value *value);

  class Iterator {
  public:
    class Entry {
    public:
      Entry() : key(NULL), value(NULL) { }
      Value *key, *value;
    };
    inline Iterator(HashMap *dict);
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

template <> class ref_traits<HashMap> : public ref_traits<Object> {
public:
  eHashMapFields(DECLARE_REF_FIELD, 0)
  inline Data *get(ref<Value> key);
  inline void set(Heap &heap, ref<Value> key, ref<Value> value);
  inline uword size();
};

DEFINE_REF_CLASS(HashMap);


// -------------------
// --- L a m b d a ---
// -------------------

#define eLambdaFields(VISIT, arg)                                    \
  VISIT(Value,      code,          Code,         arg)                \
  VISIT(Value,      constant_pool, ConstantPool, arg)                \
  VISIT(Value,      tree,          Tree,         arg)                \
  VISIT(Tuple,      outers,        Outers,       arg)                \
  VISIT(Context,    context,       Context,      arg)

class Lambda : public Object {
public:
  DECLARE_FIELD(uword, argc);
  DECLARE_FIELD(uword, max_stack_height);
  eLambdaFields(DECLARE_OBJECT_FIELD, 0)

  allocation<Lambda> clone(Heap &heap);
  void ensure_compiled(Runtime &runtime, Method *holder);

  static const uword kArgcOffset           = Object::kHeaderSize;
  static const uword kMaxStackHeightOffset = kArgcOffset + kPointerSize;
  static const uword kCodeOffset           = kMaxStackHeightOffset + kPointerSize;
  static const uword kConstantPoolOffset   = kCodeOffset + kPointerSize;
  static const uword kTreeOffset           = kConstantPoolOffset + kPointerSize;
  static const uword kOutersOffset         = kTreeOffset + kPointerSize;
  static const uword kContextOffset        = kOutersOffset + kPointerSize;
  static const uword kSize                 = kContextOffset + kPointerSize;
};

template <> class ref_traits<Lambda> : public ref_traits<Object> {
public:
  eLambdaFields(DECLARE_REF_FIELD, 0)
  void ensure_compiled(Runtime &runtime, ref<Method> holder);
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

#define eVoidFields(VISIT, arg)
class Void : public Singleton { };

#define eNullFields(VISIT, arg)
class Null : public Singleton { };

#define eTrueFields(VISIT, arg)
class True : public Bool { };

#define eFalseFields(VISIT, arg)
class False : public Bool { };


// -----------------------
// --- P r o t o c o l ---
// -----------------------

#define eProtocolFields(VISIT, arg)                          \
  VISIT(Immediate, name,    Name,    arg)                            \
  VISIT(Tuple,     methods, Methods, arg)                            \
  VISIT(Value,     super,   Super,   arg)

class Protocol : public Object {
public:
  eProtocolFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kNameOffset    = Object::kHeaderSize;
  static const uword kMethodsOffset = kNameOffset + kPointerSize;
  static const uword kSuperOffset   = kMethodsOffset + kPointerSize;
  static const uword kSize          = kSuperOffset + kPointerSize;
};

template <> class ref_traits<Protocol> : public ref_traits<Object> {
public:
  eProtocolFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Protocol);


// -------------------
// --- L a y o u t ---
// -------------------

#define eLayoutFields(VISIT, arg)                            \
  VISIT(Immediate, protocol, Protocol, arg)                          \
  VISIT(Tuple,     methods,  Methods,  arg)

class Layout : public Object {
public:
  DECLARE_FIELD(InstanceType, instance_type);
  eLayoutFields(DECLARE_OBJECT_FIELD, 0)

  bool is_empty();
  allocation<Layout> clone(Heap &heap);

  IF_DEBUG(static uword tag_of(Data *value));
  static string name_for(InstanceType type);
  static inline uword size_for(InstanceType type);

  static const char *layout_name(uword tag);
  static const uword kInstanceTypeOffset       = Object::kHeaderSize;
  static const uword kProtocolOffset           = kInstanceTypeOffset + kPointerSize;
  static const uword kMethodsOffset            = kProtocolOffset + kPointerSize;
  static const uword kHeaderSize               = kMethodsOffset + kPointerSize;
};


template <> class ref_traits<Layout> {
public:
  eLayoutFields(DECLARE_REF_FIELD, 0)
};


DEFINE_REF_CLASS(Layout);


class SimpleLayout : public Layout {
public:
  static const uword kSize = Layout::kHeaderSize;
};


class InstanceLayout : public Layout {
public:
  DECLARE_FIELD(uword, instance_field_count);

  static const uword kInstanceFieldCountOffset = Layout::kHeaderSize;
  static const uword kSize                     = kInstanceFieldCountOffset + kPointerSize;
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
  inline uword type();
  inline uword payload();
  enum Type {
    FIRST_SIGNAL_TYPE = -1
#define DECLARE_SIGNAL_TYPE(n, Name, info) , s##Name
eSignalTypes(DECLARE_SIGNAL_TYPE)
#undef DECLARE_SIGNAL_TYPE
  };
};

class AllocationFailed : public Signal {
public:
  static inline AllocationFailed *make(int size);
};

#define eFatalErrorTypes(VISIT)                                      \
  VISIT(Unexpected) VISIT(OutOfMemory) VISIT(Initialization)

class FatalError : public Signal {
public:
  enum Type {
    __first_fatal_error = -1
#define DECLARE_INTERNAL_ERROR_TYPE(Name) , fe##Name
eFatalErrorTypes(DECLARE_INTERNAL_ERROR_TYPE)
#undef DECLARE_INTERNAL_ERROR_TYPE
  };
  inline Type fatal_error_type();
  static inline FatalError *make(Type type);
  static string get_name_for(Type type);
};

class TypeMismatch : public Signal {
public:
  inline InstanceType expected();
  inline InstanceType found();
  static inline TypeMismatch *make(InstanceType expected, InstanceType found);
};

class Nothing : public Signal {
public:
  static inline Nothing *make();
};

class Success : public Signal {
public:
  static inline Success *make();
};

class StackOverflow : public Signal {
public:
  inline uword height();
  static inline StackOverflow *make(uword height);
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