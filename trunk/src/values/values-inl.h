#ifndef _HEAP_VALUES_INL
#define _HEAP_VALUES_INL

#include "compiler/ast.h"
#include "heap/pointer-inl.h"
#include "heap/ref-inl.h"
#include "runtime/context.h"
#include "utils/array-inl.h"
#include "utils/checks.h"
#include "utils/smart-ptrs-inl.h"
#include "utils/types-inl.h"
#include "values/values.h"

namespace neutrino {


// ---------------------------
// --- V a l u e   I n f o ---
// ---------------------------

/**
 * The ValueInfo class has a specialization for each type of object
 * which gives static access to various information about the class.
 */
template <class C> class ValueInfo { };

#define SPECIALIZE_VALUE_INFO(n, Name, info)                         \
template <> class ValueInfo<Name> {                                  \
public:                                                              \
  static const InstanceType kTag = t##Name;                          \
};
eDeclaredTypes(SPECIALIZE_VALUE_INFO)
#undef SPECIALIZE_VALUE_INFO


// ---------------------------
// --- P r e d i c a t e s ---
// ---------------------------

template <class To, class From>
inline To *cast(From *from) {
  ASSERT_IS_C(cnCastError, To, from);
  return static_cast<To*>(from);
}

#ifdef DEBUG
template <class To, class From>
inline To *gc_safe_cast(From *from) {
  GC_SAFE_CHECK_IS_C(cnCastError, To, from);
  return static_cast<To*>(from);
}
#endif

template <class To, class From>
inline ref<To> cast(ref<From> from) {
  ASSERT_IS_C(cnCastError, To, *from);
  return ref<To>(reinterpret_cast<To**>(from.cell()));
}

template <>
inline bool is<Data>(Data *val) {
  return true;
}

template <>
inline bool is<Smi>(Data *val) {
  return ValuePointer::has_smi_tag(val);
}

template <>
inline bool is<Object>(Data *val) {
  return ValuePointer::has_object_tag(val);
}

template <>
inline bool is<Forwarder>(Data *val) {
  return ValuePointer::has_forwarder_tag(val);
}

template <>
inline bool is<Immediate>(Data *val) {
  return is<Object>(val) || is<Smi>(val);
}

template <>
inline bool is<Value>(Data *val) {
  return !ValuePointer::has_signal_tag(val);
}

template <>
inline bool is<SyntaxTree>(Data *val) {
  if (!is<Object>(val)) return false;
  switch (cast<Object>(val)->type()) {
#define MAKE_CASE(n, Name, info) case t##Name: return true;
eSyntaxTreeTypes(MAKE_CASE)
#undef MAKE_CASE
    default: return false;
  }
}

#define DEFINE_QUERY(n, Name, info)                                  \
  template <>                                                        \
  inline bool is<Name>(Data *val) {                                  \
    return is<Object>(val)                                           \
        && cast<Object>(val)->layout()->instance_type() == t##Name;  \
  }
eObjectTypes(DEFINE_QUERY)
#undef DEFINE_QUERY

#ifdef DEBUG

#define DELEGATE_GC_SAFE_IS(Type)                                    \
  template <>                                                        \
  inline bool gc_safe_is<Type>(Data *val) {                          \
    return is<Type>(val);                                            \
  }

DELEGATE_GC_SAFE_IS(Immediate)
DELEGATE_GC_SAFE_IS(Object)
DELEGATE_GC_SAFE_IS(Value)
DELEGATE_GC_SAFE_IS(Smi)

#undef DELEGATE_GC_SAFE_IS

template <>
inline bool gc_safe_is<Bool>(Data *val) {
  if (!gc_safe_is<Object>(val)) return false;
  InstanceType type = gc_safe_cast<Object>(val)->gc_safe_type();
  return (type == tTrue) || (type == tFalse);
}

template <>
inline bool gc_safe_is<SyntaxTree>(Data *val) {
  if (!gc_safe_is<Object>(val)) return false;
  switch (gc_safe_cast<Object>(val)->gc_safe_type()) {
#define MAKE_CASE(n, Name, info) case t##Name: return true;
eSyntaxTreeTypes(MAKE_CASE)
#undef MAKE_CASE
    default: return false;
  }
}

#define DEFINE_QUERY(n, Name, info)                                  \
  template <>                                                        \
  inline bool gc_safe_is<Name>(Data *val) {                          \
    return is<Object>(val)                                           \
        && cast<Object>(val)->gc_safe_layout()->instance_type() == t##Name; \
  }
eObjectTypes(DEFINE_QUERY)
#undef DEFINE_QUERY

#endif


template <>
inline bool is<AbstractBuffer>(Data *val) {
  return is<Code>(val) || is<Buffer>(val);
}

template <>
inline bool is<AbstractTuple>(Data *val) {
  return is<Tuple>(val) || is<Array>(val);
}

template <>
inline bool is<Singleton>(Data *val) {
  UNREACHABLE();
  return false;
}

template <>
inline bool is<Root>(Data *val) {
  UNREACHABLE();
  return false;
}

template <>
inline bool is<Bool>(Data *val) {
  if (!is<Object>(val)) return false;
  InstanceType type = cast<Object>(val)->type();
  return (type == tTrue) || (type == tFalse);
}

template <>
inline bool is<Signal>(Data *val) {
  return ValuePointer::has_signal_tag(val);
}

template <>
inline bool is<ForwardPointer>(Data *val) {
  return ValuePointer::has_signal_tag(val);
}

#define DEFINE_SIGNAL_QUERY(n, Name, info)                           \
  template <>                                                        \
  inline bool is<Name>(Data *val) {                                  \
    return is<Signal>(val)                                           \
        && cast<Signal>(val)->type() == Signal::s##Name;             \
  }
eSignalTypes(DEFINE_SIGNAL_QUERY)
#undef DEFINE_SIGNAL_QUERY


// -----------------------------
// --- C o n v e r s i o n s ---
// -----------------------------

/**
 * This is the slow case in the 'to' function.  It gets called if the
 * object is not immediately an instance of C.
 */
template <class C> Data *convert_object(Value *obj) {
  ASSERT(!is<C>(obj));
  if (is<Forwarder>(obj)) {
    return to<C>(cast<Forwarder>(obj)->descriptor()->target());
  } else {
    return TypeMismatch::make(ValueInfo<C>::kTag, obj->type());
  }
}

#define DEFINE_CONVERTER(n, Name, info)                              \
  template <>                                                        \
  inline Data *to<Name>(Value *val) {                                \
    return is<Name>(val) ? val : convert_object<Name>(val);          \
  }
eDeclaredTypes(DEFINE_CONVERTER)
#undef DEFINE_CONVERTER

// All values can be converted to immediates so to<Immediate> can't
// fail.
inline Immediate *deref(Value *value) {
  return cast<Immediate>(to<Immediate>(value));
}

// -----------------
// --- V a l u e ---
// -----------------

InstanceType Value::type() {
  if (is<Smi>(this)) {
    return tSmi;
  } else if (is<Forwarder>(this)) {
    return tForwarder;
  } else {
    uword result = cast<Object>(this)->layout()->instance_type();
    return static_cast<InstanceType>(result);
  }
}

#ifdef DEBUG
InstanceType Data::gc_safe_type() {
  if (is<ForwardPointer>(this)) {
    return cast<ForwardPointer>(this)->target()->type();
  } else if (is<Smi>(this)) {
    return tSmi;
  } else {
    uword result = cast<Object>(this)->gc_safe_layout()->instance_type();
    return static_cast<InstanceType>(result);
  }
}
#endif

string ref_traits<Immediate>::to_string() {
  return open(this)->to_string();
}

InstanceType ref_traits<Value>::type() {
  return open(this)->type();
}

// -------------------------------------
// --- A c c e s s o r   M a c r o s ---
// -------------------------------------

#define DEFINE_ACCESSOR(T, Class, name, Name)                        \
T &Class::name() {                                                   \
  return ValuePointer::access_field<T>(this, Class::k##Name##Offset);\
}

#define DEFINE_REF_GETTER(T, Class, name)                            \
ref<T> ref_traits<Class>::name(RefStack &refs) {                     \
  return refs.new_ref(open(this)->name());                           \
}

#define DEFINE_SETTER(T, Class, name, Name)                          \
void Class::set_##name(T value) {                                    \
  ValuePointer::set_field<T>(this, Class::k##Name##Offset, value);   \
}

#define DEFINE_ACCESSORS(T, Class, name, Name)                       \
  DEFINE_ACCESSOR(T, Class, name, Name)                              \
  DEFINE_SETTER(T, Class, name, Name)

#define DEFINE_FIELD_ACCESSORS(T, name, Name, Class)                 \
  DEFINE_ACCESSOR(T*, Class, name, Name)                             \
  DEFINE_SETTER(T*, Class, name, Name)                               \
  DEFINE_REF_GETTER(T, Class, name)

#define DEFINE_ALL_ACCESSORS(n, Name, name)                          \
  e##Name##Fields(DEFINE_FIELD_ACCESSORS, Name)
eBoilerplateAccessors(DEFINE_ALL_ACCESSORS)
#undef DEFINE_ALL_ACCESSORS


// ---------------------------------
// --- S m a l l   I n t e g e r ---
// ---------------------------------

Smi *Smi::from_int(word value) {
  return ValuePointer::tag_as_smi(value);
}

word Smi::value() {
  return ValuePointer::value_of(this);
}


// -------------------------
// --- F o r w a r d e r ---
// -------------------------

Forwarder *Forwarder::to(ForwarderDescriptor *obj) {
  return ValuePointer::tag_as_forwarder(ValuePointer::address_of(obj));
}

ForwarderDescriptor *Forwarder::descriptor() {
  return cast<ForwarderDescriptor>(ValuePointer::tag_as_object(ValuePointer::target_of(this)));
}


// -------------------
// --- O b j e c t ---
// -------------------

#ifdef DEBUG
Layout *Object::gc_safe_layout() {
  Data *header = this->header();
  if (is<ForwardPointer>(header)) {
    return cast<ForwardPointer>(header)->target()->layout();
  } else {
    return reinterpret_cast<Layout*>(header);
  }
}
#endif

DEFINE_FIELD_ACCESSORS(Layout, layout, Header, Object)
DEFINE_ACCESSORS(Data*, Object, header, Header)


// -------------------
// --- S t r i n g ---
// -------------------

DEFINE_ACCESSORS(uword, String, length, Length)

char &String::get(uword index) {
  ASSERT_C(cnOutOfBounds, index < length());
  return ValuePointer::access_direct<char>(this, String::kHeaderSize + sizeof(char) * index);
}

void String::set(uword index, char value) {
  ASSERT_C(cnOutOfBounds, index < length());
  return ValuePointer::set_field<char>(this, String::kHeaderSize + sizeof(char) * index, value);
}

uword String::size_for(uword chars) {
  uword raw_size = kHeaderSize + sizeof(char) * chars;
  return ValuePointer::align(raw_size);
}

array<char> ref_traits<String>::c_str() {
  return open(this)->c_str();
}


// -----------------
// --- S t a c k ---
// -----------------

DEFINE_ACCESSORS(uword,         Stack, height,     Height)
DEFINE_ACCESSORS(word*,         Stack, fp,         Fp)
DEFINE_ACCESSORS(word*,         Stack, top_marker, TopMarker)
DEFINE_ACCESSORS(Stack::Status, Stack, status,     Status)

uword Stack::size_for(uword height) {
  return kHeaderSize + height * kWordSize;
}

word *Stack::bottom() {
  return &ValuePointer::access_direct<word>(this, Stack::kHeaderSize);
}


bounded_ptr<word> Stack::bound(word *ptr) {
  return NEW_BOUNDED_PTR(ptr, bottom(), bottom() + height());
}


array<word> Stack::buffer() {
  word *start = &ValuePointer::access_direct<word>(this, Stack::kHeaderSize);
  return NEW_ARRAY(start, height());
}


// -----------------------
// --- I n s t a n c e ---
// -----------------------

Value *&Instance::get_field(uword index) {
  ASSERT(index < gc_safe_layout()->instance_field_count());
  return ValuePointer::access_field<Value*>(this, Instance::kHeaderSize + kPointerSize * index);
}

void Instance::set_field(uword index, Value *value) {
  ASSERT(index < gc_safe_layout()->instance_field_count());
  return ValuePointer::set_field<Value*>(this, Instance::kHeaderSize + kPointerSize * index, value);
}

uword Instance::size_for(uword fields) {
  return Instance::kHeaderSize + fields * kPointerSize;
}


// -----------------
// --- T u p l e ---
// -----------------

DEFINE_ACCESSORS(uword, AbstractTuple, length, Length)

Value *&AbstractTuple::get(uword index) {
  ASSERT_C(cnOutOfBounds, index < length());
  return ValuePointer::access_field<Value*>(this, AbstractTuple::kHeaderSize + kPointerSize * index);
}

array<Value*> AbstractTuple::buffer() {
  return NEW_ARRAY(&ValuePointer::access_direct<Value*>(this, AbstractTuple::kHeaderSize), length());
}

void AbstractTuple::set(uword index, Value *value) {
  ASSERT_C(cnOutOfBounds, index < length());
  return ValuePointer::set_field<Value*>(this, AbstractTuple::kHeaderSize + kPointerSize * index, value);
}

uword AbstractTuple::size_for(uword elms) {
  return Tuple::kHeaderSize + elms * kPointerSize;
}

bool AbstractTuple::is_empty() {
  return length() == 0;
}

uword ref_traits<AbstractTuple>::length() {
  return open(this)->length();
}

ref<Value> ref_traits<AbstractTuple>::get(RefStack &refs, uword index) {
  return refs.new_ref(open(this)->get(index));
}

void ref_traits<AbstractTuple>::set(uword index, ref<Value> value) {
  open(this)->set(index, *value);
}


// ---------------------------
// --- D i c t i o n a r y ---
// ---------------------------

ref<Value> ref_traits<HashMap>::get(RefStack &refs, ref<Value> value) {
  Data *result = open(this)->get(*value);
  if (is<Nothing>(result)) return ref<Value>();
  else return refs.new_ref(cast<Value>(result));
}

void ref_traits<HashMap>::set(Heap &heap, ref<Value> key, ref<Value> value) {
  open(this)->set(heap, *key, *value);
}

uword ref_traits<HashMap>::size() {
  return open(this)->size();
}

HashMap::Iterator::Iterator(HashMap *dict)
    : table_(dict->table())
    , index_(0) {
}

bool HashMap::Iterator::next(HashMap::Iterator::Entry *entry) {
  if (index() < table()->length()) {
    entry->key = table()->get(index());
    entry->value = table()->get(index() + 1);
    index_ += 2;
    return true;
  } else {
    return false;
  }
}


// -------------------
// --- L a m b d a ---
// -------------------

DEFINE_ACCESSORS(uword, Lambda, argc, Argc)
DEFINE_ACCESSORS(uword, Lambda, max_stack_height, MaxStackHeight)


// ---------------------
// --- C h a n n e l ---
// ---------------------

DEFINE_ACCESSORS(void*, Channel, proxy, Proxy)


// -----------------------------------------------
// --- F o r w a r d e r   D e s c r i p t o r ---
// -----------------------------------------------

Forwarder::Type &ForwarderDescriptor::type() {
  return ValuePointer::access_field<Forwarder::Type>(this, ForwarderDescriptor::kTypeOffset);
}

void ForwarderDescriptor::set_type(Forwarder::Type value) {
  ASSERT_EQ_C(cnForwarderState, Forwarder::fwOpen, type());
  set_raw_type(value);
}

void ForwarderDescriptor::set_raw_type(Forwarder::Type value) {
  ValuePointer::set_field<Forwarder::Type>(this, ForwarderDescriptor::kTypeOffset, value);
}

Value *&ForwarderDescriptor::target() {
  return ValuePointer::access_field<Value*>(this, ForwarderDescriptor::kTargetOffset);
}

void ForwarderDescriptor::set_target(Value *value) {
  ASSERT_EQ_C(cnForwarderState, Forwarder::fwOpen, type());
  ValuePointer::set_field<Value*>(this, ForwarderDescriptor::kTargetOffset, value);
}


// -------------------
// --- B u f f e r ---
// -------------------

template <typename T>
uword AbstractBuffer::size() {
  uword field = ValuePointer::access_field<uword>(this, kSizeOffset);
  return field / sizeof(T);
}

template <typename T>
void AbstractBuffer::set_size(uword size) {
  uword value = size / sizeof(T);
  ValuePointer::set_field<uword>(this, kSizeOffset, value);
}

template <typename T>
T &AbstractBuffer::at(uword index) {
  ASSERT_C(cnOutOfBounds, index < size<T>());
  return ValuePointer::access_direct<T>(this, kHeaderSize + index * sizeof(T));
}

template <typename T>
array<T> AbstractBuffer::buffer() {
  return NEW_ARRAY(&ValuePointer::access_direct<T>(this, kHeaderSize), size<T>());
}

uword AbstractBuffer::size_for(uword byte_count) {
  uword raw_size = kHeaderSize + byte_count;
  return ValuePointer::align(raw_size);
}


// ---------------
// --- C o d e ---
// ---------------

uint16_t &Code::at(uword index) {
  return AbstractBuffer::at<uint16_t>(index);
}

array<uint16_t> Code::buffer() {
  return AbstractBuffer::buffer<uint16_t>();
}

uword Code::length() {
  return AbstractBuffer::size<uint16_t>();
}

uint16_t &ref_traits<Code>::at(uword index) {
  return open(this)->at(index);
}

uword ref_traits<Code>::length() {
  return open(this)->length();
}


// -----------------
// --- C l a s s ---
// -----------------

DEFINE_ACCESSORS(InstanceType, Layout, instance_type, InstanceType)
DEFINE_ACCESSORS(uword, Layout, instance_field_count, InstanceFieldCount)


// ---------------------
// --- S i g n a l s ---
// ---------------------

uword Signal::type() {
  return ValuePointer::signal_type(this);
}

uword Signal::payload() {
  return ValuePointer::signal_payload(this);
}

AllocationFailed *AllocationFailed::make(int size) {
  Signal *result = ValuePointer::tag_as_signal(sAllocationFailed, size);
  return cast<AllocationFailed>(result);
}

InternalError *InternalError::make(int code) {
  Signal *result = ValuePointer::tag_as_signal(sInternalError, code);
  return cast<InternalError>(result);
}

TypeMismatch *TypeMismatch::make(InstanceType expected, InstanceType found) {
  STATIC_CHECK(ValuePointer::kSignalPayloadSize >= 16);
  ASSERT_LT(expected, (1 << 8));
  ASSERT_LT(found, (1 << 8));
  Signal *result = ValuePointer::tag_as_signal(sTypeMismatch, expected << 8 | found);
  return cast<TypeMismatch>(result);
}

InstanceType TypeMismatch::expected() {
  return static_cast<InstanceType>(payload() >> 8);
}

InstanceType TypeMismatch::found() {
  return static_cast<InstanceType>(payload() & 0xFF);
}

Nothing *Nothing::make() {
  return cast<Nothing>(ValuePointer::tag_as_signal(sNothing, 0));
}

StackOverflow *StackOverflow::make() {
  return cast<StackOverflow>(ValuePointer::tag_as_signal(sStackOverflow, 0));
}

ForwardPointer *ForwardPointer::make(Object *obj) {
  return reinterpret_cast<ForwardPointer*>(ValuePointer::tag_as_signal(ValuePointer::address_of(obj)));
}

Object* ForwardPointer::target() {
  return ValuePointer::tag_as_object(reinterpret_cast<address>(ValuePointer::un_signal_tag(this)));
}

} // namespace neutrino

#endif // _HEAP_VALUES_INL
