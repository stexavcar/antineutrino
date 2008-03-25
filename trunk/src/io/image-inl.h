#ifndef _IO_IMAGE_INL
#define _IO_IMAGE_INL

#include "heap/pointer-inl.h"
#include "io/image.h"
#include "runtime/runtime-inl.h"
#include "utils/types-inl.h"
#include "values/values-inl.h"

namespace neutrino {

template <class C> class ImageValueInfo { };

#define SPECIALIZE_VALUE_INFO(n, NAME, Name, info)                   \
class Image##Name;                                                   \
template <> class ImageValueInfo<Image##Name> {                      \
public:                                                              \
  static const InstanceType kTag = NAME##_TYPE;                      \
};
FOR_EACH_DECLARED_TYPE(SPECIALIZE_VALUE_INFO)
#undef SPECIALIZE_VALUE_INFO

// ---------------------
// --- Q u e r i e s ---
// ---------------------

template <class C>
static inline C *image_raw_cast(ImageData *val) {
  ASSERT(is<C>(val));
  return reinterpret_cast<C*>(val);
}

template <>
static inline bool is<ImageObject>(ImageData *value) {
  return ValuePointer::has_object_tag(value);
}

template <>
static inline bool is<ImageSmi>(ImageData *value) {
  return ValuePointer::has_smi_tag(value);
}

template <>
static inline bool is<ImageValue>(ImageData *value) {
  return is<ImageSmi>(value) || is<ImageObject>(value);
}

template <>
static inline bool is<ImageForwardPointer>(ImageData *data) {
  return ValuePointer::has_signal_tag(data);
}

template <>
static inline bool is<ImageSyntaxTree>(ImageData *data) {
  if (!is<ImageObject>(data)) return false;
  switch (image_raw_cast<ImageObject>(data)->type()) {
#define MAKE_CASE(n, NAME, Name, info) case NAME##_TYPE: return true;
FOR_EACH_SYNTAX_TREE_TYPE(MAKE_CASE)
#undef MAKE_CASE
      return true;
    default:
      return false;
  }
}

#define DEFINE_IMAGE_OBJECT_QUERY(Class, CLASS)                      \
  template <>                                                        \
  static inline bool is<Image##Class>(ImageData *value) {            \
    return is<ImageObject>(value)                                    \
        && image_raw_cast<ImageObject>(value)->type() == CLASS##_TYPE;\
  }

DEFINE_IMAGE_OBJECT_QUERY(String,              STRING)
DEFINE_IMAGE_OBJECT_QUERY(Code,                CODE)
DEFINE_IMAGE_OBJECT_QUERY(Tuple,               TUPLE)
DEFINE_IMAGE_OBJECT_QUERY(Lambda,              LAMBDA)
DEFINE_IMAGE_OBJECT_QUERY(Dictionary,          DICTIONARY)
DEFINE_IMAGE_OBJECT_QUERY(Task,                TASK)
DEFINE_IMAGE_OBJECT_QUERY(Stack,               STACK)
DEFINE_IMAGE_OBJECT_QUERY(Layout,              LAYOUT)
DEFINE_IMAGE_OBJECT_QUERY(Protocol,            PROTOCOL)
DEFINE_IMAGE_OBJECT_QUERY(Context,             CONTEXT)
DEFINE_IMAGE_OBJECT_QUERY(Method,              METHOD)
DEFINE_IMAGE_OBJECT_QUERY(Signature,           SIGNATURE)
DEFINE_IMAGE_OBJECT_QUERY(Root,                SINGLETON)
DEFINE_IMAGE_OBJECT_QUERY(ForwarderDescriptor, FORWARDER_DESCRIPTOR)
DEFINE_IMAGE_OBJECT_QUERY(Selector,            SELECTOR)
#define DEFINE_SYNTAX_TREE_QUERY(n, NAME, Name, info) DEFINE_IMAGE_OBJECT_QUERY(Name, NAME)
FOR_EACH_SYNTAX_TREE_TYPE(DEFINE_SYNTAX_TREE_QUERY)
#undef DEFINE_SYNTAX_TREE_QUERY

InstanceType ImageData::type() {
  if (is<ImageSmi>(this)) return SMI_TYPE;
  else return image_raw_cast<ImageObject>(this)->type();
}

template <class C>
static inline C *image_cast(ImageData *val, ImageLoadInfo &info,
    const char *location) {
  if (!is<C>(val)) {
    info.type_mismatch(ImageValueInfo<C>::kTag, val->type(), location);
    return 0;
  } else {
    return image_raw_cast<C>(val);
  }
}

template <class To, class From>
inline To *safe_cast(From *from, ImageLoadInfo &info,
    const char *location) {
  if (!is<To>(from)) {
    info.type_mismatch(ValueInfo<To>::kTag, from->type(), location);
    return 0;
  } else {
    return cast<To>(from);
  }
}

// ---------------------------------------
// --- F o r w a r d   P o i n t e r s ---
// ---------------------------------------

ImageData *ImageObject::header() {
  uword offset = ValuePointer::offset_of(this) + ImageObject_LayoutOffset;
  return reinterpret_cast<ImageData*>(Image::current().heap()[offset]);
}

bool ImageObject::has_been_migrated() {
  return is<ImageForwardPointer>(header());
}

void ImageObject::point_forward(Object *obj) {
  ASSERT(!is<ImageForwardPointer>(header()));
  uword offset = ValuePointer::offset_of(this) + ImageObject_LayoutOffset;
  ImageForwardPointer *pointer = ImageForwardPointer::to(obj);
  Image::current().heap()[offset] = reinterpret_cast<uword>(pointer);
}

Value *ImageValue::forward_pointer() {
  if (is<ImageSmi>(this)) {
    return image_raw_cast<ImageSmi>(this)->forward_pointer();
  } else if (is<ImageRoot>(this)) {
    return Runtime::current().roots().get(image_raw_cast<ImageRoot>(this)->index());
  } else {
    return image_raw_cast<ImageObject>(this)->forward_pointer();
  }
}

Smi *ImageSmi::forward_pointer() {
  return Smi::from_int(value());
}

Object *ImageObject::forward_pointer() {
  uword offset = ValuePointer::offset_of(this) + ImageObject_LayoutOffset;
  ImageData *value = ImageData::from(Image::current().heap()[offset]);
  ImageForwardPointer *pointer = image_raw_cast<ImageForwardPointer>(value);
  return pointer->target();
}

ImageForwardPointer *ImageForwardPointer::to(Object *obj) {
  uword value = ValuePointer::tag_as_signal(ValuePointer::address_of(obj));
  return reinterpret_cast<ImageForwardPointer*>(value);
}

Object *ImageForwardPointer::target() {
  return ValuePointer::tag_as_object(reinterpret_cast<address>(ValuePointer::un_signal_tag(this)));
}


// -----------------
// --- I m a g e ---
// -----------------

Image &Image::current() {
  ASSERT(current_ != NULL);
  return *current_;
}

Image::Scope::Scope(Image &image)
    : previous_(current_) {
  current_ = &image;
}

Image::Scope::~Scope() {
  current_ = previous_;
}

ImageIterator::ImageIterator(Image &image)
    : cursor_(0)
    , limit_(image.heap_size()) {
}

bool ImageIterator::has_next() {
  return cursor() < limit();
}

void ImageIterator::reset() {
  cursor_ = 0;
}

ImageObject *ImageIterator::next() {
  uword object_ptr = ValuePointer::tag_offset_as_object(cursor());
  ImageObject *obj = image_raw_cast<ImageObject>(ImageData::from(object_ptr));
  cursor_ += obj->size_in_image();
  return obj;
}

ImageData *ImageData::from(uword addr) {
  return reinterpret_cast<ImageData*>(addr);
}

ImageData *ImageData::from(Immediate *obj) {
  return reinterpret_cast<ImageData*>(obj);
}

word ImageSmi::value() {
  return ValuePointer::value_of(this);
}

#define DEFINE_RAW_GETTER(T, Class, name, Name)                      \
  T Image##Class::name() {                                           \
    uword offset = ValuePointer::offset_of(this) + Image##Class##_##Name##Offset;\
    return Image::current().heap()[offset];                          \
  }

#define DEFINE_GETTER(T, Class, name, Name)                          \
  Image##T *Image##Class::name(ImageLoadInfo &info, const char *location) {\
    uword offset = ValuePointer::offset_of(this) + Image##Class##_##Name##Offset;\
    ImageData *data = ImageData::from(Image::current().heap()[offset]);\
    return image_cast<Image##T>(data, info, location);               \
  }

// --- S t r i n g ---

DEFINE_RAW_GETTER(uword, String, length, Length)

uword ImageString::at(uword index) {
  ASSERT(index < length());
  uword offset = ValuePointer::offset_of(this) + ImageString_HeaderSize + index;
  return Image::current().heap()[offset];
}

// --- T u p l e ---

DEFINE_RAW_GETTER(uword, Tuple, length, Length)

ImageValue *ImageTuple::at(uword index) {
  ASSERT(index < length());
  uword offset = ValuePointer::offset_of(this) + ImageCode_HeaderSize + index;
  ImageData *data = ImageData::from(Image::current().heap()[offset]);
  return image_raw_cast<ImageValue>(data);
}

// --- C o d e ---

DEFINE_RAW_GETTER(uword, Code, length, Length)

uword ImageCode::at(uword index) {
  ASSERT(index < length());
  uword offset = ValuePointer::offset_of(this) + ImageCode_HeaderSize + index;
  return Image::current().heap()[offset];
}

DEFINE_GETTER    (Tuple,            Protocol,              methods,         Methods)
DEFINE_GETTER    (Value,            Protocol,              name,            Name)
DEFINE_GETTER    (Value,            Protocol,              super,           Super)
DEFINE_RAW_GETTER(uword,            Layout,                instance_type,   InstanceType)
DEFINE_GETTER    (Tuple,            Layout,                methods,         Methods)
DEFINE_GETTER    (Value,            Layout,                protocol,        Protocol)
DEFINE_RAW_GETTER(uword,            Lambda,                argc,            Argc)
DEFINE_GETTER    (Value,            Lambda,                code,            Code)
DEFINE_GETTER    (Value,            Lambda,                literals,        Literals)
DEFINE_GETTER    (SyntaxTree,       Lambda,                tree,            Tree)
DEFINE_GETTER    (Context,          Lambda,                context,         Context)
DEFINE_GETTER    (Selector,         Method,                selector,        Selector)
DEFINE_GETTER    (Signature,        Method,                signature,       Signature)
DEFINE_GETTER    (Lambda,           Method,                lambda,          Lambda)
DEFINE_GETTER    (Tuple,            Signature,             parameters,      Parameters)
DEFINE_GETTER    (Tuple,            Dictionary,            table,           Table)
DEFINE_RAW_GETTER(uword,            Root,                  index,           Index)
DEFINE_GETTER    (Value,            LiteralExpression,     value,           Value)
DEFINE_GETTER    (SyntaxTree,       InvokeExpression,      receiver,        Receiver)
DEFINE_GETTER    (Selector,         InvokeExpression,      selector,        Selector)
DEFINE_GETTER    (Value,            InvokeExpression,      arguments,       Arguments)
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
DEFINE_GETTER    (String,           ProtocolExpression,    name,            Name)
DEFINE_GETTER    (Tuple,            ProtocolExpression,    methods,         Methods)
DEFINE_GETTER    (Value,            ProtocolExpression,    super,           Super)
DEFINE_GETTER    (SyntaxTree,       ReturnExpression,      value,           Value)
DEFINE_GETTER    (SyntaxTree,       YieldExpression,       value,           Value)
DEFINE_GETTER    (Selector,         MethodExpression,      selector,        Selector)
DEFINE_GETTER    (LambdaExpression, MethodExpression,      lambda,          Lambda)
DEFINE_GETTER    (Value,            MethodExpression,      is_static,       IsStatic)
DEFINE_GETTER    (Parameters,       LambdaExpression,      parameters,      Parameters)
DEFINE_GETTER    (SyntaxTree,       LambdaExpression,      body,            Body)
DEFINE_GETTER    (Tuple,            SequenceExpression,    expressions,     Expressions)
DEFINE_GETTER    (Tuple,            TupleExpression,       values,          Values)
DEFINE_GETTER    (Value,            Symbol,                name,            Name)
DEFINE_GETTER    (SyntaxTree,       QuoteExpression,       value,           Value)
DEFINE_GETTER    (Tuple,            QuoteExpression,       unquotes,        Unquotes)
DEFINE_GETTER    (SyntaxTree,       QuoteTemplate,         value,           Value)
DEFINE_GETTER    (Tuple,            QuoteTemplate,         unquotes,        Unquotes)
DEFINE_RAW_GETTER(uword,            UnquoteExpression,     index,           Index)
DEFINE_GETTER    (String,           GlobalExpression,      name,            Name)
DEFINE_RAW_GETTER(uword,            BuiltinCall,           argc,            Argc)
DEFINE_RAW_GETTER(uword,            BuiltinCall,           index,           Index)
DEFINE_GETTER    (Smi,              ExternalCall,          argc,            Argc)
DEFINE_GETTER    (String,           ExternalCall,          name,            Name)
DEFINE_GETTER    (Tuple,            InterpolateExpression, terms,           Terms)
DEFINE_GETTER    (Symbol,           LocalDefinition,       symbol,          Symbol)
DEFINE_GETTER    (SyntaxTree,       LocalDefinition,       value,           Value)
DEFINE_GETTER    (SyntaxTree,       LocalDefinition,       body,            Body)
DEFINE_GETTER    (Symbol,           Assignment,            symbol,          Symbol)
DEFINE_GETTER    (SyntaxTree,       Assignment,            value,           Value)
DEFINE_GETTER    (Stack,            Task,                  stack,           Stack)
DEFINE_GETTER    (Value,            ForwarderDescriptor,   target,          Target)
DEFINE_GETTER    (Tuple,            Arguments,             arguments,       Arguments)
DEFINE_GETTER    (Tuple,            Arguments,             keyword_indices, KeywordIndices)
DEFINE_GETTER    (Smi,              Parameters,            position_count,  PositionCount)
DEFINE_GETTER    (Tuple,            Parameters,            parameters,      Parameters)
DEFINE_GETTER    (Value,            Selector,              name,            Name)
DEFINE_GETTER    (Smi,              Selector,              argc,            Argc)
DEFINE_GETTER    (Tuple,            Selector,              keywords,        Keywords)
DEFINE_GETTER    (Value,            Selector,              is_accessor,     IsAccessor)
DEFINE_GETTER    (LambdaExpression, TaskExpression,        lambda,          Lambda)

#undef DEFINE_RAW_GETTER
#undef DEFINE_GETTER

} // neutrino

#endif // _IO_IMAGE_INL
