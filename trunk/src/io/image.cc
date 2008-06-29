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

Image::Image(list<word> data)
    : data_(data) { }

Image::~Image() {
  data().dispose();
}

// -------------------------------------
// --- I m a g e   L o a d   I n f o ---
// -------------------------------------

void ImageLoadStatus::type_mismatch(InstanceType expected, InstanceType found,
    const char *location) {
  status = lsTypeMismatch;
  error_info.type_mismatch.expected = expected;
  error_info.type_mismatch.found = found;
  error_info.type_mismatch.location = location;
}

void ImageLoadStatus::invalid_image() {
  status = lsInvalidImage;
}

void ImageLoadStatus::invalid_root_count(word expected, word found) {
  status = lsRootCount;
  error_info.root_count.expected = expected;
  error_info.root_count.found = found;
}

void ImageLoadStatus::invalid_magic_number(uword found) {
  status = lsInvalidMagic;
  error_info.magic.found = found;
}

void ImageLoadStatus::invalid_version(uword found) {
  status = lsInvalidVersion;
  error_info.version.found = found;
}

MAKE_ENUM_INFO_HEADER(ImageLoadStatus::Status)
#define MAKE_ENTRY(Name) MAKE_ENUM_INFO_ENTRY(ImageLoadStatus::ls##Name)
eImageLoadStatuses(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

// ---------------------
// --- L o a d i n g ---
// ---------------------

void Image::initialize(ImageContext &context) {
  if (data().length() < kHeaderSize) {
    context.status().invalid_image();
    return;
  }
  if (data_[kMagicNumberOffset] != kMagicNumber) {
    context.status().invalid_magic_number(data_[kMagicNumberOffset]);
    return;
  }
  if (data_[kVersionOffset] != Image::kCurrentVersion) {
    context.status().invalid_version(data_[kVersionOffset]);
    return;
  }
  if (data_[kRootCountOffset] != static_cast<word>(Roots::kExternalCount)) {
    context.status().invalid_root_count(Roots::kExternalCount, data_[kRootCountOffset]);
    return;
  }
  uword heap_size = data_[kHeapSizeOffset];
  if (heap_size > data().length() - kHeaderSize) {
    context.status().invalid_image();
    return;
  }
  word *heap_start = data().start() + kHeaderSize;
  heap_ = list<word>(heap_start, heap_size);
}

Data *Image::load(ImageContext &info) {
  DisallowGarbageCollection disallow(info.runtime().heap().memory());
  // Make shallow copies in heap
  ImageIterator<FixedHeap, FixedHeap::Data> iter(heap());
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
  FObject *start = reinterpret_cast<FObject*>(ValuePointer::tag_as_object(heap().start()));
  FData *roots_val = cook_value(start, reinterpret_cast<RawFValue*>(data_[kRootsOffset]));
  FTuple *roots_img = image_cast<FTuple>(roots_val, info, "<roots>");
  if (info.has_error()) return Nothing::make();
  return safe_cast<Tuple>(roots_img->forward_pointer(), info, "<roots>");
}

void Image::copy_object_shallow(FObject *obj, ImageContext &info) {
  Heap &heap = info.runtime().heap();
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
      String *str = heap.new_string(length).value();
      for (uword i = 0; i < length; i++)
        str->set(i, img->get(i));
      obj->point_forward(str);
      break;
    }
    case tCode: {
      FCode *img = image_raw_cast<FCode>(obj);
      uword length = img->length();
      Code *code = heap.new_code(length).value();
      for (uword i = 0; i < length; i++)
        code->set(i, img->get(i));
      obj->point_forward(code);
      break;
    }
    case tTuple: {
      FTuple *img = image_raw_cast<FTuple>(obj);
      uword length = img->length();
      Tuple *tuple = heap.new_tuple(length).value();
      obj->point_forward(tuple);
      break;
    }
    case tLayout: {
      // Because of the rule that layouts must be moved before their
      // instances this layout may already have been moved.
      if (obj->has_been_migrated()) return;
      FLayout *img = image_raw_cast<FLayout>(obj);
      InstanceType instance_type = static_cast<InstanceType>(img->instance_type());
      Layout *layout = heap.allocate_empty_layout(instance_type).value();
      obj->point_forward(layout);
      break;
    }
    case tContext: {
      FContext *img = image_raw_cast<FContext>(obj);
      use(img);
      Context *context = heap.new_context().value();
      obj->point_forward(context);
      break;
    }
    case tLambda: {
      FLambda *img = image_raw_cast<FLambda>(obj);
      uword argc = img->argc();
      Lambda *lambda = heap.allocate_lambda(argc).value();
      obj->point_forward(lambda);
      break;
    }
    case tRoot: {
      break;
    }
#define MAKE_CASE(n, Name, name)                                     \
    case t##Name: {                                                  \
      Name *heap_obj = heap.allocate_##name().value();               \
      obj->point_forward(heap_obj);                                  \
      break;                                                         \
    }
eBoilerplateShallowCopy(MAKE_CASE)
#undef MAKE_CASE
    default:
      UNHANDLED(InstanceType, type);
      break;
  }
  if (type_info.has_layout())
    obj->forward_pointer()->set_layout(type_info.layout);
}

void Image::fixup_shallow_object(FObject *obj, ImageContext &info) {
#define TRANSFER_FIELD(Type, field, Field, arg)                      \
  FImmediate *field##_img = img->field(info, #arg "." #field);       \
  if (info.has_error()) return;                                      \
  Type *field##_value = safe_cast<Type>(field##_img->forward_pointer(info), info, #arg "." #field); \
  if (info.has_error()) return;                                      \
  heap_obj->set_##field(field##_value);
  InstanceType type = obj->type();
  switch (type) {
    case tTuple: {
      FTuple *img = image_raw_cast<FTuple>(obj);
      Tuple *tuple = cast<Tuple>(img->forward_pointer());
      uword length = tuple->length();
      for (uword i = 0; i < length; i++)
        tuple->set(i, img->at(i)->forward_pointer(info));
      break;
    }
    case tLambda: {
      FLambda *img = image_raw_cast<FLambda>(obj);
      Lambda *heap_obj = cast<Lambda>(img->forward_pointer());
      TRANSFER_FIELD(Value, code, Code, Lambda);
      TRANSFER_FIELD(LambdaExpression, tree, Tree, Lambda);
      TRANSFER_FIELD(Context, context, Context, Lambda);
      heap_obj->set_constant_pool(img->literals(info, "Lambda.constant_pool")->forward_pointer(info));
      heap_obj->set_outers(info.runtime().heap().roots().empty_tuple());
      break;
    }
    case tChannel: {
      FChannel *img = image_raw_cast<FChannel>(obj);
      Channel *heap_obj = cast<Channel>(img->forward_pointer());
      TRANSFER_FIELD(String, name, Name, Channel);
      break;
    }
    case tBuiltinCall: {
      FBuiltinCall *img = image_raw_cast<FBuiltinCall>(obj);
      BuiltinCall *expr = cast<BuiltinCall>(img->forward_pointer());
      expr->set_argc(img->argc());
      expr->set_index(img->index());
      break;
    }
    case tSymbol: {
      FSymbol *img = image_raw_cast<FSymbol>(obj);
      Symbol *heap_obj = cast<Symbol>(img->forward_pointer());
      TRANSFER_FIELD(Immediate, name, Name, Symbol);
      heap_obj->set_data(info.runtime().roots().nuhll());
      break;
    }
    case tContext: {
      break;
    }
    case tString: case tCode: case tRoot:
      // Nothing to fix
      break;
#define MAKE_CASE(n, Name, name)                                     \
    case t##Name: {                                                  \
      F##Name *img = image_cast<F##Name>(obj, info, #Name);          \
      if (info.has_error()) return;                                  \
      Name *heap_obj = cast<Name>(img->forward_pointer());           \
      use(heap_obj);                                                 \
      e##Name##Fields(TRANSFER_FIELD, Name)                          \
      break;                                                         \
    }
eBoilerplateFixupShallow(MAKE_CASE)
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
#define MAKE_CASE(n, Name, name) case t##Name: return F##Name##_Size;
eBoilerplateSizeInImage(MAKE_CASE)
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
  return size_for(length());
}

uword FString::size_for(uword chars) {
  return FString_HeaderSize + chars;
}

uword FCode::code_size_in_image() {
  return FCode_HeaderSize + length();
}

uword FTuple::size_for(uword elms) {
  return FTuple_HeaderSize + elms;
}

uword FTuple::tuple_size_in_image() {
  return size_for(length());
}

}
