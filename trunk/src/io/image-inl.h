#ifndef _IO_IMAGE_INL
#define _IO_IMAGE_INL

#include "heap/pointer-inl.h"
#include "io/image.h"
#include "runtime/runtime.h"
#include "utils/types-inl.h"

namespace neutrino {

// ---------------------
// --- Q u e r i e s ---
// ---------------------

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
  switch (image_cast<ImageObject>(data)->type()) {
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
        && image_cast<ImageObject>(value)->type() == CLASS##_TYPE;   \
  }

DEFINE_IMAGE_OBJECT_QUERY(String,     STRING)
DEFINE_IMAGE_OBJECT_QUERY(Code,       CODE)
DEFINE_IMAGE_OBJECT_QUERY(Tuple,      TUPLE)
DEFINE_IMAGE_OBJECT_QUERY(Lambda,     LAMBDA)
DEFINE_IMAGE_OBJECT_QUERY(Dictionary, DICTIONARY)
DEFINE_IMAGE_OBJECT_QUERY(Task,       TASK)
DEFINE_IMAGE_OBJECT_QUERY(Stack,      STACK)
DEFINE_IMAGE_OBJECT_QUERY(Class,      CLASS)
DEFINE_IMAGE_OBJECT_QUERY(Protocol,   PROTOCOL)
DEFINE_IMAGE_OBJECT_QUERY(Method,     METHOD)
DEFINE_IMAGE_OBJECT_QUERY(Root,       SINGLETON)
#define DEFINE_SYNTAX_TREE_QUERY(n, NAME, Name, info) DEFINE_IMAGE_OBJECT_QUERY(Name, NAME)
FOR_EACH_SYNTAX_TREE_TYPE(DEFINE_SYNTAX_TREE_QUERY)
#undef DEFINE_SYNTAX_TREE_QUERY

template <class C>
static inline C *image_cast(ImageData *val) {
  ASSERT(is<C>(val));
  return reinterpret_cast<C*>(val);
}

// ---------------------------------------
// --- F o r w a r d   P o i n t e r s ---
// ---------------------------------------

void ImageObject::point_forward(Object *obj) {
  uint32_t offset = ValuePointer::offset_of(this) + ImageObject_TypeOffset;
  ImageForwardPointer *pointer = ImageForwardPointer::to(obj);
  Image::current().heap()[offset] = reinterpret_cast<uint32_t>(pointer);
}

Value *ImageValue::forward_pointer() {
  if (is<ImageSmi>(this)) {
    return image_cast<ImageSmi>(this)->forward_pointer();
  } else if (is<ImageRoot>(this)) {
    return Runtime::current().roots().get(image_cast<ImageRoot>(this)->index());
  } else {
    return image_cast<ImageObject>(this)->forward_pointer();
  }
}

Smi *ImageSmi::forward_pointer() {
  return Smi::from_int(value());
}

Object *ImageObject::forward_pointer() {
  uint32_t offset = ValuePointer::offset_of(this) + ImageObject_TypeOffset;
  ImageData *value = ImageData::from(Image::current().heap()[offset]);
  ImageForwardPointer *pointer = image_cast<ImageForwardPointer>(value);
  return pointer->target();
}

ImageForwardPointer *ImageForwardPointer::to(Object *obj) {
  uint32_t value = ValuePointer::tag_as_signal(ValuePointer::address_of(obj));
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
  uint32_t object_ptr = ValuePointer::tag_offset_as_object(cursor());
  ImageObject *obj = image_cast<ImageObject>(ImageData::from(object_ptr));
  cursor_ += obj->size_in_image();
  return obj;
}

ImageData *ImageData::from(uint32_t addr) {
  return reinterpret_cast<ImageData*>(addr);
}

int32_t ImageSmi::value() {
  return ValuePointer::value_of(this);
}

#define DEFINE_RAW_GETTER(T, Class, name, Name)                      \
  T Image##Class::name() {                                           \
    uint32_t offset = ValuePointer::offset_of(this) + Image##Class##_##Name##Offset;\
    return Image::current().heap()[offset];                          \
  }

#define DEFINE_GETTER(T, Class, name, Name)                          \
  Image##T *Image##Class::name() {                                   \
    uint32_t offset = ValuePointer::offset_of(this) + Image##Class##_##Name##Offset;\
    ImageData *data = ImageData::from(Image::current().heap()[offset]);\
    return image_cast<Image##T>(data);                               \
  }

// --- S t r i n g ---

DEFINE_RAW_GETTER(uint32_t, String, length, Length)

uint32_t ImageString::at(uint32_t index) {
  ASSERT(index < length());
  uint32_t offset = ValuePointer::offset_of(this) + ImageString_HeaderSize + index;
  return Image::current().heap()[offset];
}

// --- T u p l e ---

DEFINE_RAW_GETTER(uint32_t, Tuple, length, Length)

ImageValue *ImageTuple::at(uint32_t index) {
  ASSERT(index < length());
  uint32_t offset = ValuePointer::offset_of(this) + ImageCode_HeaderSize + index;
  ImageData *data = ImageData::from(Image::current().heap()[offset]);
  return image_cast<ImageValue>(data);
}

// --- C o d e ---

DEFINE_RAW_GETTER(uint32_t, Code, length, Length)

uint32_t ImageCode::at(uint32_t index) {
  ASSERT(index < length());
  uint32_t offset = ValuePointer::offset_of(this) + ImageCode_HeaderSize + index;
  return Image::current().heap()[offset];
}

DEFINE_GETTER    (Tuple,            Protocol,              methods,       Methods)
DEFINE_GETTER    (Value,            Protocol,              name,          Name)
DEFINE_RAW_GETTER(uint32_t,         Class,                 instance_type, InstanceType)
DEFINE_GETTER    (Tuple,            Class,                 methods,       Methods)
DEFINE_GETTER    (Value,            Class,                 super,         Super)
DEFINE_GETTER    (Value,            Class,                 name,          Name)
DEFINE_RAW_GETTER(uint32_t,         Lambda,                argc,          Argc)
DEFINE_GETTER    (Value,            Lambda,                code,          Code)
DEFINE_GETTER    (Value,            Lambda,                literals,      Literals)
DEFINE_GETTER    (SyntaxTree,       Lambda,                tree,          Tree)
DEFINE_GETTER    (String,           Method,                name,          Name)
DEFINE_GETTER    (Lambda,           Method,                lambda,        Lambda)
DEFINE_GETTER    (Tuple,            Dictionary,            table,         Table)
DEFINE_RAW_GETTER(uint32_t,         Root,                  index,         Index)
DEFINE_GETTER    (Value,            LiteralExpression,     value,         Value)
DEFINE_GETTER    (SyntaxTree,       InvokeExpression,      receiver,      Receiver)
DEFINE_GETTER    (String,           InvokeExpression,      name,          Name)
DEFINE_GETTER    (Tuple,            InvokeExpression,      arguments,     Arguments)
DEFINE_GETTER    (SyntaxTree,       InstantiateExpression, receiver,      Receiver)
DEFINE_GETTER    (String,           InstantiateExpression, name,          Name)
DEFINE_GETTER    (Tuple,            InstantiateExpression, arguments,     Arguments)
DEFINE_GETTER    (Tuple,            InstantiateExpression, terms,         Terms)
DEFINE_GETTER    (String,           RaiseExpression,       name,          Name)
DEFINE_GETTER    (Tuple,            RaiseExpression,       arguments,     Arguments)
DEFINE_GETTER    (String,           OnClause,              name,          Name)
DEFINE_GETTER    (LambdaExpression, OnClause,              lambda,        Lambda)
DEFINE_GETTER    (SyntaxTree,       DoOnExpression,        value,         Value)
DEFINE_GETTER    (Tuple,            DoOnExpression,        clauses,       Clauses)
DEFINE_GETTER    (SyntaxTree,       CallExpression,        receiver,      Receiver)
DEFINE_GETTER    (SyntaxTree,       CallExpression,        function,      Function)
DEFINE_GETTER    (Tuple,            CallExpression,        arguments,     Arguments)
DEFINE_GETTER    (SyntaxTree,       ConditionalExpression, condition,     Condition)
DEFINE_GETTER    (SyntaxTree,       ConditionalExpression, then_part,     ThenPart)
DEFINE_GETTER    (SyntaxTree,       ConditionalExpression, else_part,     ElsePart)
DEFINE_GETTER    (String,           ClassExpression,       name,          Name)
DEFINE_GETTER    (Tuple,            ClassExpression,       methods,       Methods)
DEFINE_GETTER    (Value,            ClassExpression,       super,         Super)
DEFINE_GETTER    (SyntaxTree,       ReturnExpression,      value,         Value)
DEFINE_GETTER    (String,           MethodExpression,      name,          Name)
DEFINE_GETTER    (LambdaExpression, MethodExpression,      lambda,        Lambda)
DEFINE_GETTER    (Tuple,            LambdaExpression,      params,        Params)
DEFINE_GETTER    (SyntaxTree,       LambdaExpression,      body,          Body)
DEFINE_GETTER    (Tuple,            SequenceExpression,    expressions,   Expressions)
DEFINE_GETTER    (Tuple,            TupleExpression,       values,        Values)
DEFINE_GETTER    (Value,            Symbol,                name,          Name)
DEFINE_GETTER    (SyntaxTree,       QuoteExpression,       value,         Value)
DEFINE_GETTER    (Tuple,            QuoteExpression,       unquotes,      Unquotes)
DEFINE_GETTER    (SyntaxTree,       QuoteTemplate,         value,         Value)
DEFINE_GETTER    (Tuple,            QuoteTemplate,         unquotes,      Unquotes)
DEFINE_RAW_GETTER(uint32_t,         UnquoteExpression,     index,         Index)
DEFINE_GETTER    (String,           GlobalExpression,      name,          Name)
DEFINE_RAW_GETTER(uint32_t,         BuiltinCall,           argc,          Argc)
DEFINE_RAW_GETTER(uint32_t,         BuiltinCall,           index,         Index)
DEFINE_GETTER    (Tuple,            InterpolateExpression, terms,         Terms)
DEFINE_GETTER    (Symbol,           LocalDefinition,       symbol,        Symbol)
DEFINE_GETTER    (SyntaxTree,       LocalDefinition,       value,         Value)
DEFINE_GETTER    (SyntaxTree,       LocalDefinition,       body,          Body)
DEFINE_GETTER    (Stack,            Task,                  stack,         Stack)

#undef DEFINE_RAW_GETTER
#undef DEFINE_GETTER

} // neutrino

#endif // _IO_IMAGE_INL
