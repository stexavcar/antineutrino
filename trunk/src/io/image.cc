#include "compiler/ast-inl.h"
#include "heap/memory-inl.h"
#include "heap/roots-inl.h"
#include "io/image-inl.h"
#include "io/in-stream-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/list-inl.h"
#include "values/values-inl.h"

namespace neutrino {

// 0xFABACEAE
// image size
// root count
// word_1
// word_2
// ...

Image *Image::current_ = NULL;

Image::Image(uword size, word *data)
    : size_(size)
    , data_(data)
    , heap_(NULL) { }

Image::~Image() {
  delete[] data_;
}

// -------------------------------------
// --- I m a g e   L o a d   I n f o ---
// -------------------------------------

void ImageLoadInfo::type_mismatch(InstanceType expected, InstanceType found,
    const char *location) {
  status = TYPE_MISMATCH;
  error_info.type_mismatch.expected = expected;
  error_info.type_mismatch.found = found;
  error_info.type_mismatch.location = location;
}

void ImageLoadInfo::invalid_image() {
  status = INVALID_IMAGE;
}

void ImageLoadInfo::invalid_root_count(word expected, word found) {
  status = ROOT_COUNT;
  error_info.root_count.expected = expected;
  error_info.root_count.found = found;
}

void ImageLoadInfo::invalid_magic_number(uword found) {
  status = INVALID_MAGIC;
  error_info.magic.found = found;
}

void ImageLoadInfo::invalid_version(uword found) {
  status = INVALID_VERSION;
  error_info.version.found = found;
}

MAKE_ENUM_INFO_HEADER(ImageLoadInfo::Status)
#define MAKE_ENTRY(NAME) MAKE_ENUM_INFO_ENTRY(ImageLoadInfo::NAME)
FOR_EACH_IMAGE_LOAD_STATUS(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

// ---------------------
// --- L o a d i n g ---
// ---------------------

void Image::initialize(ImageLoadInfo &info) {
  if (size_ < kHeaderSize) {
    info.invalid_image();
    return;
  }
  if (data_[kMagicNumberOffset] != kMagicNumber) {
    info.invalid_magic_number(data_[kMagicNumberOffset]);
    return;
  }
  if (data_[kVersionOffset] != Image::kCurrentVersion) {
    info.invalid_version(data_[kVersionOffset]);
    return;
  }
  if (data_[kRootCountOffset] != static_cast<word>(Roots::kCount)) {
    info.invalid_root_count(Roots::kCount, data_[kRootCountOffset]);
    return;
  }
  heap_size_ = data_[kHeapSizeOffset];
  if (heap_size_ > size_) {
    info.invalid_image();
    return;
  }
  heap_ = data_ + kHeaderSize;
}

Data *Image::load(ImageLoadInfo &info) {
  DisallowGarbageCollection disallow(Runtime::current().heap().memory());
  Image::Scope scope(*this);
  // Make shallow copies in heap
  ImageIterator iter(*this);
  while (iter.has_next()) {
    copy_object_shallow(iter.next(), info);
    if (info.has_error())
      return Nothing::make();
  }
  // Fixup the copies
  iter.reset();
  while (iter.has_next()) {
    fixup_shallow_object(iter.next(), info);
    if (info.has_error())
      return Nothing::make();
  }
  FObject *start = reinterpret_cast<FObject*>(ValuePointer::tag_as_object(heap()));
  FData *roots_val = cook_value(start, reinterpret_cast<RawFValue*>(data_[kRootsOffset]));
  FTuple *roots_img = image_cast<FTuple>(roots_val, info, "<roots>");
  if (info.has_error()) return Nothing::make();
  return safe_cast<Tuple>(roots_img->forward_pointer(), info, "<roots>");
}

void Image::copy_object_shallow(FObject *obj, ImageLoadInfo &info) {
  Heap &heap = Runtime::current().heap();
  FData *header = obj->header();
  // Always migrate layout objects before the objects they describe.
  // It is useful to be able to make the assumption that the layout
  // has been moved before any instances because the layout field in
  // the image will be overwritten with a forward pointer.
  if (is<FLayout>(header) && !image_raw_cast<FLayout>(header)->has_been_migrated()) {
    copy_object_shallow(image_raw_cast<FLayout>(header), info);
    if (info.has_error()) return;
  }
  TypeInfo type_info;
  obj->type_info(&type_info);
  InstanceType type = type_info.type;
  switch (type) {
    case tString: {
      FString *img = image_raw_cast<FString>(obj);
      uword length = img->length();
      String *str = cast<String>(heap.new_string(length));
      for (uword i = 0; i < length; i++)
        str->at(i) = img->at(i);
      obj->point_forward(str);
      break;
    }
    case tCode: {
      FCode *img = image_raw_cast<FCode>(obj);
      uword length = img->length();
      Code *code = cast<Code>(heap.new_code(length));
      for (uword i = 0; i < length; i++)
        code->at(i) = img->at(i);
      obj->point_forward(code);
      break;
    }
    case tTuple: {
      FTuple *img = image_raw_cast<FTuple>(obj);
      uword length = img->length();
      Tuple *tuple = cast<Tuple>(heap.new_tuple(length));
      obj->point_forward(tuple);
      break;
    }
    case tLayout: {
      // Because of the rule that layouts must be moved before their
      // instances this layout may already have been moved.
      if (obj->has_been_migrated()) return;
      FLayout *img = image_raw_cast<FLayout>(obj);
      InstanceType instance_type = static_cast<InstanceType>(img->instance_type());
      Layout *layout = cast<Layout>(heap.allocate_empty_layout(instance_type));
      obj->point_forward(layout);
      break;
    }
    case tContext: {
      FContext *img = image_raw_cast<FContext>(obj);
      USE(img);
      Context *context = cast<Context>(heap.new_context());
      obj->point_forward(context);
      break;
    }
    case tBuiltinCall: {
      BuiltinCall *heap_obj = cast<BuiltinCall>(heap.allocate_builtin_call());
      obj->point_forward(heap_obj);
      break;
    }
    case tLambda: {
      FLambda *img = image_raw_cast<FLambda>(obj);
      uword argc = img->argc();
      Lambda *lambda = cast<Lambda>(heap.allocate_lambda(argc));
      obj->point_forward(lambda);
      break;
    }
    case tUnquoteExpression: {
      UnquoteExpression *heap_obj = cast<UnquoteExpression>(heap.allocate_unquote_expression());
      obj->point_forward(heap_obj);
      break;
    }
    case tRoot: {
      break;
    }
#define MAKE_CASE(n, NAME, Name, name)                               \
    case t##Name: {                                                  \
      Name *heap_obj = cast<Name>(heap.allocate_##name());           \
      obj->point_forward(heap_obj);                                  \
      break;                                                         \
    }
FOR_EACH_GENERATABLE_TYPE(MAKE_CASE)
#undef MAKE_CASE
    default:
      UNHANDLED(InstanceType, type);
      break;
  }
  if (type_info.has_layout())
    obj->forward_pointer()->set_layout(type_info.layout);
}

void Image::fixup_shallow_object(FObject *obj, ImageLoadInfo &info) {
#define TRANSFER_FIELD(Type, field, Field, arg)                      \
  FImmediate *field##_img = img->field(info, #arg "." #field);       \
  if (info.has_error()) return;                                      \
  Type *field##_value = safe_cast<Type>(field##_img->forward_pointer(), info, #arg "." #field); \
  if (info.has_error()) return;                                      \
  heap_obj->set_##field(field##_value);
  InstanceType type = obj->type();
  switch (type) {
    case tTuple: {
      FTuple *img = image_raw_cast<FTuple>(obj);
      Tuple *tuple = cast<Tuple>(img->forward_pointer());
      uword length = tuple->length();
      for (uword i = 0; i < length; i++)
        tuple->set(i, img->at(i)->forward_pointer());
      break;
    }
    case tLambda: {
      FLambda *img = image_raw_cast<FLambda>(obj);
      Lambda *heap_obj = cast<Lambda>(img->forward_pointer());
      TRANSFER_FIELD(Value, code, Code, Lambda);
      TRANSFER_FIELD(LambdaExpression, tree, Tree, Lambda);
      TRANSFER_FIELD(Context, context, Context, Lambda);
      heap_obj->set_constant_pool(img->literals(info, "Lambda.constant_pool")->forward_pointer());
      heap_obj->set_outers(Runtime::current().heap().roots().empty_tuple());
      break;
    }
    case tLayout: {
      FLayout *img = image_raw_cast<FLayout>(obj);
      Layout *layout = cast<Layout>(img->forward_pointer());
      layout->set_methods(cast<Tuple>(img->methods(info, "Layout.methods")->forward_pointer()));
      layout->set_protocol(cast<Immediate>(img->protocol(info, "Layout.protocol")->forward_pointer()));
      break;
    }
    case tBuiltinCall: {
      FBuiltinCall *img = image_raw_cast<FBuiltinCall>(obj);
      BuiltinCall *expr = cast<BuiltinCall>(img->forward_pointer());
      expr->set_argc(img->argc());
      expr->set_index(img->index());
      break;
    }
    case tUnquoteExpression: {
      FUnquoteExpression *img = image_raw_cast<FUnquoteExpression>(obj);
      UnquoteExpression *expr = cast<UnquoteExpression>(img->forward_pointer());
      expr->set_index(img->index());
      break;
    }
    case tContext: {
      break;
    }
    case tString: case tCode: case tRoot:
      // Nothing to fix
      break;
#define MAKE_CASE(n, NAME, Name, name)                               \
    case t##Name: {                                                  \
      F##Name *img = image_cast<F##Name>(obj, info, #Name);          \
      if (info.has_error()) return;                                  \
      Name *heap_obj = cast<Name>(img->forward_pointer());           \
      USE(heap_obj);                                                 \
      FOR_EACH_##NAME##_FIELD(TRANSFER_FIELD, Name)                  \
      break;                                                         \
    }
FOR_EACH_GENERATABLE_TYPE(MAKE_CASE)
#undef MAKE_CASE
#undef TRANSFER_FIELD
    default:
      UNHANDLED(InstanceType, type);
      break;
  }
  IF_PARANOID(if (type != tRoot) obj->forward_pointer()->validate());
}

InstanceType FObject::type() {
  TypeInfo type_info;
  this->type_info(&type_info);
  return type_info.type;
}

void FObject::type_info(TypeInfo *result) {
  FData *data = header();
  if (is<FSmi>(data)) {
    word value = image_raw_cast<FSmi>(data)->value();
    result->type = static_cast<InstanceType>(value);
  } else if (is<FForwardPointer>(data)) {
    Object *target = image_raw_cast<FForwardPointer>(data)->target();
    result->layout = target->layout();
    result->type = target->layout()->instance_type();
  } else if (is<FLayout>(data)) {
    FLayout *layout = image_raw_cast<FLayout>(data);
    word value = layout->instance_type();
    result->type = static_cast<InstanceType>(value);
    if (layout->has_been_migrated())
      result->layout = cast<Layout>(layout->forward_pointer());
  } else {
    UNREACHABLE();
  }
}

// -----------------------------
// --- O b j e c t   s i z e ---
// -----------------------------

uword FObject::size_in_image() {
  uword type = this->type();
  switch (type) {
    case tLambda:
      return FLambda_Size;
    case tLayout:
      return FLayout_Size;
    case tContext:
      return FContext_Size;
    case tRoot:
      return FRoot_Size;
    case tBuiltinCall:
      return FBuiltinCall_Size;
    case tUnquoteExpression:
      return FUnquoteExpression_Size;
#define MAKE_CASE(n, NAME, Name, name)                               \
    case t##Name: return F##Name##_Size;
FOR_EACH_GENERATABLE_TYPE(MAKE_CASE)
#undef MAKE_CASE
    case tString:
      return image_raw_cast<FString>(this)->string_size_in_image();
    case tCode:
      return image_raw_cast<FCode>(this)->code_size_in_image();
    case tTuple:
      return image_raw_cast<FTuple>(this)->tuple_size_in_image();
    default:
      UNHANDLED(InstanceType, type);
      return 0;
  }
}

uword FString::string_size_in_image() {
  return FString_HeaderSize + length();
}

uword FCode::code_size_in_image() {
  return FCode_HeaderSize + length();
}

uword FTuple::tuple_size_in_image() {
  return FTuple_HeaderSize + length();
}

}
