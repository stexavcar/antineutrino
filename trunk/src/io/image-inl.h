#ifndef _IO_IMAGE_INL
#define _IO_IMAGE_INL

#include "heap/pointer-inl.h"
#include "io/image.h"
#include "runtime/runtime-inl.h"
#include "utils/types-inl.h"
#include "values/values-inl.h"

namespace neutrino {

template <class C> class FImmediateInfo { };

#define SPECIALIZE_VALUE_INFO(n, Name, info)                         \
class F##Name;                                                       \
template <> class FImmediateInfo<F##Name> {                          \
public:                                                              \
  static const InstanceType kTag = t##Name;                          \
};
eDeclaredTypes(SPECIALIZE_VALUE_INFO)
#undef SPECIALIZE_VALUE_INFO

// ---------------------
// --- Q u e r i e s ---
// ---------------------

template <class C>
static inline C *image_raw_cast(FData *val) {
  ASSERT(is<C>(val));
  return reinterpret_cast<C*>(val);
}

template <>
static inline bool is<FObject>(FData *value) {
  return ValuePointer::has_object_tag(value);
}

template <>
static inline bool is<FSmi>(FData *value) {
  return ValuePointer::has_smi_tag(value);
}

template <>
static inline bool is<FImmediate>(FData *value) {
  return is<FSmi>(value) || is<FObject>(value);
}

template <>
static inline bool is<FForwardPointer>(FData *data) {
  return ValuePointer::has_signal_tag(data);
}

template <>
static inline bool is<FSyntaxTree>(FData *data) {
  if (!is<FObject>(data)) return false;
  switch (image_raw_cast<FObject>(data)->type()) {
#define MAKE_CASE(n, Name, info) case t##Name: return true;
eSyntaxTreeTypes(MAKE_CASE)
#undef MAKE_CASE
      return true;
    default:
      return false;
  }
}

#define DEFINE_IMAGE_OBJECT_QUERY(Name)                              \
  template <>                                                        \
  static inline bool is<F##Name>(FData *value) {                     \
    return is<FObject>(value)                                        \
        && image_raw_cast<FObject>(value)->type() == t##Name;        \
  }

DEFINE_IMAGE_OBJECT_QUERY(String)
DEFINE_IMAGE_OBJECT_QUERY(Code)
DEFINE_IMAGE_OBJECT_QUERY(Tuple)
DEFINE_IMAGE_OBJECT_QUERY(Lambda)
DEFINE_IMAGE_OBJECT_QUERY(Dictionary)
DEFINE_IMAGE_OBJECT_QUERY(Task)
DEFINE_IMAGE_OBJECT_QUERY(Stack)
DEFINE_IMAGE_OBJECT_QUERY(Layout)
DEFINE_IMAGE_OBJECT_QUERY(Protocol)
DEFINE_IMAGE_OBJECT_QUERY(Context)
DEFINE_IMAGE_OBJECT_QUERY(Method)
DEFINE_IMAGE_OBJECT_QUERY(Signature)
DEFINE_IMAGE_OBJECT_QUERY(Root)
DEFINE_IMAGE_OBJECT_QUERY(ForwarderDescriptor)
DEFINE_IMAGE_OBJECT_QUERY(Selector)
DEFINE_IMAGE_OBJECT_QUERY(Channel)
DEFINE_IMAGE_OBJECT_QUERY(Symbol)
DEFINE_IMAGE_OBJECT_QUERY(Cell)
#define DEFINE_SYNTAX_TREE_QUERY(n, Name, info) DEFINE_IMAGE_OBJECT_QUERY(Name)
eSyntaxTreeTypes(DEFINE_SYNTAX_TREE_QUERY)
#undef DEFINE_SYNTAX_TREE_QUERY

InstanceType FData::type() {
  if (is<FSmi>(this)) return tSmi;
  else return image_raw_cast<FObject>(this)->type();
}

template <class C>
static inline C *image_cast(FData *val, ImageContext &context,
    const char *location) {
  if (!is<C>(val)) {
    context.status().type_mismatch(FImmediateInfo<C>::kTag, val->type(), location);
    return 0;
  } else {
    return image_raw_cast<C>(val);
  }
}

template <class To, class From>
inline To *safe_cast(From *from, ImageContext &context,
    const char *location) {
  if (!is<To>(from)) {
    context.status().type_mismatch(ValueInfo<To>::kTag, from->type(), location);
    return 0;
  } else {
    return cast<To>(from);
  }
}

static inline RawFValue *&access_field(FObject *obj, uword field_offset) {
  RawFValue **ptr = reinterpret_cast<RawFValue**>(ValuePointer::address_of(obj));
  return ptr[field_offset];
}

template <typename T>
static inline T *&access_field(FObject *obj, uword field_offset) {
  T **ptr = reinterpret_cast<T**>(ValuePointer::address_of(obj));
  return ptr[field_offset];
}

static inline FData *cook_value(FObject *holder, RawFValue *value) {
  if (ValuePointer::has_object_tag(value)) {
    address addr = ValuePointer::address_of(holder);
    word offset = reinterpret_cast<word>(value);
    // We add the offset directly since both addr and offset are
    // word size aligned and offset is object tagged which means that
    // the result will also be object tagged
    return FData::from(reinterpret_cast<word>(addr + offset));
  } else {
    return FData::from(reinterpret_cast<word>(value));
  }
}

static inline RawFValue *uncook_value(FObject *holder, FData *data) {
  ASSERT(ValuePointer::has_smi_tag(data));
  return reinterpret_cast<RawFValue*>(data);
}

template <class C>
static inline C *get_field(FObject *obj, uword field_offset,
    ImageContext &info, const char *location) {
  RawFValue *value = access_field(obj, field_offset);
  FData *data = cook_value(obj, value);
  return image_cast<C>(data, info, location);
}

template <class C>
static inline C *get_raw_field(FObject *obj, uword field_offset) {
  RawFValue *value = access_field(obj, field_offset);
  FData *data = cook_value(obj, value);
  return image_raw_cast<C>(data);
}

// ---------------------------------------
// --- F o r w a r d   P o i n t e r s ---
// ---------------------------------------

FData *FObject::header() {
  RawFValue *value = access_field(this, FObject_LayoutOffset);
  return cook_value(this, value);
}

void FObject::set_header(FData *type) {
  access_field(this, FObject_LayoutOffset) = uncook_value(this, type);
}

bool FObject::has_been_migrated() {
  return is<FForwardPointer>(header());
}

void FObject::point_forward(Object *obj) {
  ASSERT(!is<FForwardPointer>(header()));
  FForwardPointer *pointer = FForwardPointer::to(obj);
  access_field(this, FObject_LayoutOffset) = reinterpret_cast<RawFValue*>(pointer);
}

Value *FImmediate::forward_pointer(ImageContext &info) {
  if (is<FSmi>(this)) {
    return image_raw_cast<FSmi>(this)->forward_pointer();
  } else if (is<FRoot>(this)) {
    return info.runtime().roots().get(image_raw_cast<FRoot>(this)->index());
  } else {
    return image_raw_cast<FObject>(this)->forward_pointer();
  }
}

Smi *FSmi::forward_pointer() {
  return Smi::from_int(value());
}

Object *FObject::forward_pointer() {
  FData *value = header();
  FForwardPointer *pointer = image_raw_cast<FForwardPointer>(value);
  return pointer->target();
}

FForwardPointer *FForwardPointer::to(Object *obj) {
  uword value = ValuePointer::tag_as_signal(ValuePointer::address_of(obj));
  return reinterpret_cast<FForwardPointer*>(value);
}

Object *FForwardPointer::target() {
  return ValuePointer::tag_as_object(reinterpret_cast<address>(ValuePointer::un_signal_tag(this)));
}


// -----------------
// --- I m a g e ---
// -----------------

FData *FData::from(uword addr) {
  return reinterpret_cast<FData*>(addr);
}

FData *FData::from(Immediate *obj) {
  return reinterpret_cast<FData*>(obj);
}

word FSmi::value() {
  return ValuePointer::value_of(this);
}

#define DEFINE_RAW_GETTER(T, Class, name, Name)                      \
  T F##Class::name() {                                               \
    return reinterpret_cast<word>(access_field(this, F##Class##_##Name##Offset)); \
  }

#define DEFINE_RAW_SETTER(T, Class, name, Name)                      \
  void F##Class::set_##name(T value) {                               \
    access_field(this, F##Class##_##Name##Offset) = reinterpret_cast<RawFValue*>(value); \
  }

#define DEFINE_GETTER(T, Class, name, Name)                          \
  F##T *F##Class::name(ImageContext &info, const char *location) {  \
    return get_field<F##T>(this, F##Class##_##Name##Offset, info, location); \
  }

// --- S t r i n g ---

DEFINE_RAW_GETTER(uword, String, length, Length)
DEFINE_RAW_SETTER(uword, String, length, Length)

uword FString::get(uword index) {
  ASSERT(index < length());
  return reinterpret_cast<word>(access_field(this, FString_HeaderSize + index));
}

void FString::set(uword index, uword value) {
  ASSERT(index < length());
  access_field(this, FString_HeaderSize + index) = reinterpret_cast<RawFValue*>(value);
}

// --- T u p l e ---

DEFINE_RAW_GETTER(uword, Tuple, length, Length)
DEFINE_RAW_SETTER(uword, Tuple, length, Length)

FImmediate *FTuple::at(uword index) {
  ASSERT(index < length());
  return get_raw_field<FImmediate>(this, FTuple_HeaderSize + index);
}

void FTuple::set_raw(uword index, Value *value) {
  access_field<Value>(this, FTuple_HeaderSize + index) = value;
}

Value *FTuple::get_raw(uword index) {
  return access_field<Value>(this, FTuple_HeaderSize + index);
}

// --- C o d e ---

DEFINE_RAW_GETTER(uword, Code, length, Length)

uword FCode::at(uword index) {
  ASSERT(index < length());
  return reinterpret_cast<word>(access_field(this, FCode_HeaderSize + index));
}

DEFINE_GETTER    (Tuple,            Protocol,              methods,         Methods)
DEFINE_GETTER    (Immediate,        Protocol,              name,            Name)
DEFINE_GETTER    (Immediate,        Protocol,              super,           Super)
DEFINE_RAW_GETTER(uword,            Layout,                instance_type,   InstanceType)
DEFINE_GETTER    (Tuple,            Layout,                methods,         Methods)
DEFINE_GETTER    (Immediate,        Layout,                protocol,        Protocol)
DEFINE_RAW_GETTER(uword,            Lambda,                argc,            Argc)
DEFINE_GETTER    (Immediate,        Lambda,                code,            Code)
DEFINE_GETTER    (Immediate,        Lambda,                literals,        Literals)
DEFINE_GETTER    (SyntaxTree,       Lambda,                tree,            Tree)
DEFINE_GETTER    (Context,          Lambda,                context,         Context)
DEFINE_GETTER    (String,           Channel,               name,            Name)
DEFINE_GETTER    (Selector,         Method,                selector,        Selector)
DEFINE_GETTER    (Signature,        Method,                signature,       Signature)
DEFINE_GETTER    (Lambda,           Method,                lambda,          Lambda)
DEFINE_GETTER    (Tuple,            Signature,             parameters,      Parameters)
DEFINE_GETTER    (Tuple,            Dictionary,            table,           Table)
DEFINE_RAW_GETTER(uword,            Root,                  index,           Index)
DEFINE_GETTER    (Immediate,        LiteralExpression,     value,           Value)
DEFINE_GETTER    (SyntaxTree,       InvokeExpression,      receiver,        Receiver)
DEFINE_GETTER    (Selector,         InvokeExpression,      selector,        Selector)
DEFINE_GETTER    (Immediate,        InvokeExpression,      arguments,       Arguments)
DEFINE_GETTER    (SyntaxTree,       InstantiateExpression, receiver,        Receiver)
DEFINE_GETTER    (String,           InstantiateExpression, name,            Name)
DEFINE_GETTER    (Arguments,        InstantiateExpression, arguments,       Arguments)
DEFINE_GETTER    (Tuple,            InstantiateExpression, terms,           Terms)
DEFINE_GETTER    (String,           RaiseExpression,       name,            Name)
DEFINE_GETTER    (Arguments,        RaiseExpression,       arguments,       Arguments)
DEFINE_GETTER    (String,           OnClause,              name,            Name)
DEFINE_GETTER    (LambdaExpression, OnClause,              lambda,          Lambda)
DEFINE_GETTER    (SyntaxTree,       DoOnExpression,        value,           Value)
DEFINE_GETTER    (Tuple,            DoOnExpression,        clauses,         Clauses)
DEFINE_GETTER    (SyntaxTree,       CallExpression,        receiver,        Receiver)
DEFINE_GETTER    (SyntaxTree,       CallExpression,        function,        Function)
DEFINE_GETTER    (Arguments,        CallExpression,        arguments,       Arguments)
DEFINE_GETTER    (SyntaxTree,       ConditionalExpression, condition,       Condition)
DEFINE_GETTER    (SyntaxTree,       ConditionalExpression, then_part,       ThenPart)
DEFINE_GETTER    (SyntaxTree,       ConditionalExpression, else_part,       ElsePart)
DEFINE_GETTER    (SyntaxTree,       WhileExpression,       condition,       Condition)
DEFINE_GETTER    (SyntaxTree,       WhileExpression,       body,            Body)
DEFINE_GETTER    (String,           ProtocolExpression,    name,            Name)
DEFINE_GETTER    (Tuple,            ProtocolExpression,    methods,         Methods)
DEFINE_GETTER    (Immediate,        ProtocolExpression,    super,           Super)
DEFINE_GETTER    (SyntaxTree,       ReturnExpression,      value,           Value)
DEFINE_GETTER    (SyntaxTree,       YieldExpression,       value,           Value)
DEFINE_GETTER    (Selector,         MethodExpression,      selector,        Selector)
DEFINE_GETTER    (LambdaExpression, MethodExpression,      lambda,          Lambda)
DEFINE_GETTER    (Immediate,        MethodExpression,      is_static,       IsStatic)
DEFINE_GETTER    (Parameters,       LambdaExpression,      parameters,      Parameters)
DEFINE_GETTER    (SyntaxTree,       LambdaExpression,      body,            Body)
DEFINE_GETTER    (Tuple,            SequenceExpression,    expressions,     Expressions)
DEFINE_GETTER    (Tuple,            TupleExpression,       values,          Values)
DEFINE_GETTER    (Immediate,        Symbol,                name,            Name)
DEFINE_GETTER    (SyntaxTree,       QuoteExpression,       value,           Value)
DEFINE_GETTER    (Tuple,            QuoteExpression,       unquotes,        Unquotes)
DEFINE_GETTER    (SyntaxTree,       QuoteTemplate,         value,           Value)
DEFINE_GETTER    (Tuple,            QuoteTemplate,         unquotes,        Unquotes)
DEFINE_RAW_GETTER(uword,            UnquoteExpression,     index,           Index)
DEFINE_GETTER    (String,           GlobalVariable,        name,            Name)
DEFINE_GETTER    (Symbol,           LocalVariable,         symbol,          Symbol)
DEFINE_RAW_GETTER(uword,            BuiltinCall,           argc,            Argc)
DEFINE_RAW_GETTER(uword,            BuiltinCall,           index,           Index)
DEFINE_GETTER    (Tuple,            InterpolateExpression, terms,           Terms)
DEFINE_GETTER    (Symbol,           LocalDefinition,       symbol,          Symbol)
DEFINE_GETTER    (SyntaxTree,       LocalDefinition,       value,           Value)
DEFINE_GETTER    (SyntaxTree,       LocalDefinition,       body,            Body)
DEFINE_GETTER    (Smi,              LocalDefinition,       type,            Type)
DEFINE_GETTER    (Symbol,           Assignment,            symbol,          Symbol)
DEFINE_GETTER    (SyntaxTree,       Assignment,            value,           Value)
DEFINE_GETTER    (Stack,            Task,                  stack,           Stack)
DEFINE_GETTER    (Immediate,        Cell,                  value,           Value)
DEFINE_GETTER    (Immediate,        ForwarderDescriptor,   target,          Target)
DEFINE_GETTER    (Tuple,            Arguments,             arguments,       Arguments)
DEFINE_GETTER    (Tuple,            Arguments,             keyword_indices, KeywordIndices)
DEFINE_GETTER    (Smi,              Parameters,            position_count,  PositionCount)
DEFINE_GETTER    (Tuple,            Parameters,            parameters,      Parameters)
DEFINE_GETTER    (Immediate,        Selector,              name,            Name)
DEFINE_GETTER    (Smi,              Selector,              argc,            Argc)
DEFINE_GETTER    (Tuple,            Selector,              keywords,        Keywords)
DEFINE_GETTER    (Immediate,        Selector,              is_accessor,     IsAccessor)
DEFINE_GETTER    (LambdaExpression, TaskExpression,        lambda,          Lambda)
DEFINE_GETTER    (SyntaxTree,       SuperExpression,       value,           Value)

#undef DEFINE_RAW_GETTER
#undef DEFINE_GETTER

} // neutrino

#endif // _IO_IMAGE_INL
