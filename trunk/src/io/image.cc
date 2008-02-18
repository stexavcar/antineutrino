#include "compiler/ast-inl.h"
#include "heap/memory-inl.h"
#include "heap/roots-inl.h"
#include "io/image-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/list-inl.h"
#include "values/values-inl.h"

namespace neutrino {

// 0xFABACEAE
// root count
// root_0
// root_1
// ...
// root_k
// image size
// word_1
// word_2
// ...

Image *Image::current_ = NULL;

Image::Image(uword size, uword *data)
    : size_(size)
    , data_(data)
    , heap_(0) { }

Image::~Image() {
  delete[] data_;
}

// ---------------------
// --- L o a d i n g ---
// ---------------------

bool Image::initialize() {
  if (size_ < kHeaderSize) {
    Conditions::get().error_occurred("Invalid image");
    return false;
  }
  if (data_[kMagicNumberOffset] != kMagicNumber) {
    Conditions::get().error_occurred("Invalid image");
    return false;
  }
  heap_size_ = data_[kHeapSizeOffset];
  if (heap_size_ > size_) {
    Conditions::get().error_occurred("Invalid image");
    return false;
  }
  heap_ = data_ + kHeaderSize;
  return true;
}

Tuple *Image::load() {
  DisallowGarbageCollection disallow(Runtime::current().heap().memory());
  Image::Scope scope(*this);
  // Make shallow copies in heap
  ImageIterator iter(*this);
  while (iter.has_next())
    copy_object_shallow(iter.next());
  // Fixup the copies
  iter.reset();
  while (iter.has_next())
    fixup_shallow_object(iter.next());
  ImageTuple *roots_img = image_cast<ImageTuple>(ImageValue::from(data_[kRootsOffset]));
  return cast<Tuple>(roots_img->forward_pointer());
}

void Image::copy_object_shallow(ImageObject *obj) {
  Heap &heap = Runtime::current().heap();
  ImageData *header = obj->header();
  // Always migrate layout objects before the objects they describe.
  // It is useful to be able to make the assumption that the layout
  // has been moved before any instances because the layout field in
  // the image will be overwritten with a forward pointer.
  if (is<ImageLayout>(header) && !image_cast<ImageLayout>(header)->has_been_migrated())
    copy_object_shallow(image_cast<ImageLayout>(header));
  TypeInfo type_info;
  obj->type_info(&type_info);
  InstanceType type = type_info.type;
  switch (type) {
    case STRING_TYPE: {
      ImageString *img = image_cast<ImageString>(obj);
      uword length = img->length();
      String *str = cast<String>(heap.new_string(length));
      for (uword i = 0; i < length; i++)
        str->at(i) = img->at(i);
      obj->point_forward(str);
      break;
    }
    case CODE_TYPE: {
      ImageCode *img = image_cast<ImageCode>(obj);
      uword length = img->length();
      Code *code = cast<Code>(heap.new_code(length));
      for (uword i = 0; i < length; i++)
        code->at(i) = img->at(i);
      obj->point_forward(code);
      break;
    }
    case TUPLE_TYPE: {
      ImageTuple *img = image_cast<ImageTuple>(obj);
      uword length = img->length();
      Tuple *tuple = cast<Tuple>(heap.new_tuple(length));
      obj->point_forward(tuple);
      break;
    }
    case LAMBDA_TYPE: {
      ImageLambda *img = image_cast<ImageLambda>(obj);
      uword argc = img->argc();
      Lambda *lambda = cast<Lambda>(heap.allocate_lambda(argc));
      obj->point_forward(lambda);
      break;
    }
    case LAYOUT_TYPE: {
      // Because of the rule that layouts must be moved before their
      // instances this layout may already have been moved.
      if (obj->has_been_migrated()) return;
      ImageLayout *img = image_cast<ImageLayout>(obj);
      InstanceType instance_type = static_cast<InstanceType>(img->instance_type());
      Layout *layout = cast<Layout>(heap.allocate_empty_layout(instance_type));
      obj->point_forward(layout);
      break;
    }
    case CONTEXT_TYPE: {
      ImageContext *img = image_cast<ImageContext>(obj);
      USE(img);
      Context *context = cast<Context>(heap.new_context());
      obj->point_forward(context);
      break;
    }
    case BUILTIN_CALL_TYPE: {
      BuiltinCall *heap_obj = cast<BuiltinCall>(heap.allocate_builtin_call());
      obj->point_forward(heap_obj);
      break;
    }
    case UNQUOTE_EXPRESSION_TYPE: {
      UnquoteExpression *heap_obj = cast<UnquoteExpression>(heap.allocate_unquote_expression());
      obj->point_forward(heap_obj);
      break;
    }
    case SINGLETON_TYPE: {
      // Roots (which are singleton tagged) are handled specially in
      // the Value::forward_pointer method.
      break;
    }
#define MAKE_CASE(n, NAME, Name, name)                               \
    case NAME##_TYPE: {                                              \
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

void Image::fixup_shallow_object(ImageObject *obj) {
  InstanceType type = obj->type();
  switch (type) {
    case TUPLE_TYPE: {
      ImageTuple *img = image_cast<ImageTuple>(obj);
      Tuple *tuple = cast<Tuple>(img->forward_pointer());
      uword length = tuple->length();
      for (uword i = 0; i < length; i++)
        tuple->set(i, img->at(i)->forward_pointer());
      break;
    }
    case LAMBDA_TYPE: {
      ImageLambda *img = image_cast<ImageLambda>(obj);
      Lambda *lambda = cast<Lambda>(img->forward_pointer());
      lambda->set_code(img->code()->forward_pointer());
      lambda->set_constant_pool(img->literals()->forward_pointer());
      lambda->set_tree(cast<LambdaExpression>(img->tree()->forward_pointer()));
      lambda->set_outers(Runtime::current().heap().roots().empty_tuple());
      lambda->set_context(cast<Context>(img->context()->forward_pointer()));
      break;
    }
    case LAYOUT_TYPE: {
      ImageLayout *img = image_cast<ImageLayout>(obj);
      Layout *layout = cast<Layout>(img->forward_pointer());
      layout->set_methods(cast<Tuple>(img->methods()->forward_pointer()));
      layout->set_protocol(cast<Immediate>(img->protocol()->forward_pointer()));
      break;
    }
    case BUILTIN_CALL_TYPE: {
      ImageBuiltinCall *img = image_cast<ImageBuiltinCall>(obj);
      BuiltinCall *expr = cast<BuiltinCall>(img->forward_pointer());
      expr->set_argc(img->argc());
      expr->set_index(img->index());
      break;
    }
    case UNQUOTE_EXPRESSION_TYPE: {
      ImageUnquoteExpression *img = image_cast<ImageUnquoteExpression>(obj);
      UnquoteExpression *expr = cast<UnquoteExpression>(img->forward_pointer());
      expr->set_index(img->index());
      break;
    }
    case CONTEXT_TYPE: {
      break;
    }
    case STRING_TYPE: case CODE_TYPE: case SINGLETON_TYPE:
      // Nothing to fix
      break;
#define TRANSFER_FIELD(Type, field, Field, arg)                      \
  heap_obj->set_##field(cast<Type>(img->field()->forward_pointer()));
#define MAKE_CASE(n, NAME, Name, name)                               \
    case NAME##_TYPE: {                                              \
      Image##Name *img = image_cast<Image##Name>(obj);               \
      Name *heap_obj = cast<Name>(img->forward_pointer());           \
      USE(heap_obj);                                                 \
      FOR_EACH_##NAME##_FIELD(TRANSFER_FIELD, 0)                     \
      break;                                                         \
    }
FOR_EACH_GENERATABLE_TYPE(MAKE_CASE)
#undef MAKE_CASE
#undef TRANSFER_FIELD
    default:
      UNHANDLED(InstanceType, type);
      break;
  }
  IF_PARANOID(if (type != SINGLETON_TYPE) obj->forward_pointer()->validate());
}

InstanceType ImageObject::type() {
  TypeInfo type_info;
  this->type_info(&type_info);
  return type_info.type;
}

void ImageObject::type_info(TypeInfo *result) {
  ImageData *data = header();
  if (is<ImageSmi>(data)) {
    word value = image_cast<ImageSmi>(data)->value();
    result->type = static_cast<InstanceType>(value);
  } else if (is<ImageForwardPointer>(data)) {
    Object *target = image_cast<ImageForwardPointer>(data)->target();
    result->layout = target->layout();
    result->type = target->layout()->instance_type();
  } else if (is<ImageLayout>(data)) {
    ImageLayout *layout = image_cast<ImageLayout>(data);
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

uword ImageObject::size_in_image() {
  uword type = this->type();
  switch (type) {
    case LAMBDA_TYPE:
      return ImageLambda_Size;
    case LAYOUT_TYPE:
      return ImageLayout_Size;
    case CONTEXT_TYPE:
      return ImageContext_Size;
    case SINGLETON_TYPE:
      return ImageRoot_Size;
    case BUILTIN_CALL_TYPE:
      return ImageBuiltinCall_Size;
    case UNQUOTE_EXPRESSION_TYPE:
      return ImageUnquoteExpression_Size;
#define MAKE_CASE(n, NAME, Name, name)                               \
    case NAME##_TYPE: return Image##Name##_Size;
FOR_EACH_GENERATABLE_TYPE(MAKE_CASE)
#undef MAKE_CASE
    case STRING_TYPE:
      return image_cast<ImageString>(this)->string_size_in_image();
    case CODE_TYPE:
      return image_cast<ImageCode>(this)->code_size_in_image();
    case TUPLE_TYPE:
      return image_cast<ImageTuple>(this)->tuple_size_in_image();
    default:
      UNHANDLED(InstanceType, type);
      return 0;
  }
}

uword ImageString::string_size_in_image() {
  return ImageString_HeaderSize + length();
}

uword ImageCode::code_size_in_image() {
  return ImageCode_HeaderSize + length();
}

uword ImageTuple::tuple_size_in_image() {
  return ImageTuple_HeaderSize + length();
}

}
