#include "compiler/ast-inl.h"
#include "heap/memory-inl.h"
#include "heap/roots-inl.h"
#include "heap/values-inl.h"
#include "io/image-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/list-inl.h"

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

Image::Image(uint32_t size, uint32_t *data)
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
  uint32_t type = obj->type();
  Heap &heap = Runtime::current().heap();
  switch (type) {
    case STRING_TYPE: {
      ImageString *img = image_cast<ImageString>(obj);
      uint32_t length = img->length();
      String *str = cast<String>(heap.new_string(length));
      for (uint32_t i = 0; i < length; i++)
        str->at(i) = img->at(i);
      obj->point_forward(str);
      break;
    }
    case CODE_TYPE: {
      ImageCode *img = image_cast<ImageCode>(obj);
      uint32_t length = img->length();
      Code *code = cast<Code>(heap.new_code(length));
      for (uint32_t i = 0; i < length; i++)
        code->at(i) = img->at(i);
      obj->point_forward(code);
      break;
    }
    case TUPLE_TYPE: {
      ImageTuple *img = image_cast<ImageTuple>(obj);
      uint32_t length = img->length();
      Tuple *tuple = cast<Tuple>(heap.new_tuple(length));
      obj->point_forward(tuple);
      break;
    }
    case LAMBDA_TYPE: {
      ImageLambda *img = image_cast<ImageLambda>(obj);
      uint32_t argc = img->argc();
      Lambda *lambda = cast<Lambda>(heap.allocate_lambda(argc));
      obj->point_forward(lambda);
      break;
    }
    case CLASS_TYPE: {
      ImageClass *img = image_cast<ImageClass>(obj);
      InstanceType instance_type = static_cast<InstanceType>(img->instance_type());
      Class *chlass = cast<Class>(heap.allocate_empty_class(instance_type));
      obj->point_forward(chlass);
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
}

void Image::fixup_shallow_object(ImageObject *obj) {
  uint32_t type = obj->type();
  switch (type) {
    case TUPLE_TYPE: {
      ImageTuple *img = image_cast<ImageTuple>(obj);
      Tuple *tuple = cast<Tuple>(img->forward_pointer());
      uint32_t length = tuple->length();
      for (uint32_t i = 0; i < length; i++)
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
      break;
    }
    case CLASS_TYPE: {
      ImageClass *img = image_cast<ImageClass>(obj);
      Class *chlass = cast<Class>(img->forward_pointer());
      chlass->set_methods(cast<Tuple>(img->methods()->forward_pointer()));
      if (img->super()->forward_pointer() == Smi::from_int(0))
        chlass->set_super(Runtime::current().roots().vhoid());
      else
        chlass->set_super(img->super()->forward_pointer());
      chlass->set_name(img->name()->forward_pointer());
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
  IF_PARANOID(if (obj->type() != SINGLETON_TYPE) obj->forward_pointer()->validate());
}

uint32_t ImageObject::type() {
  uint32_t offset = ValuePointer::offset_of(this) + ImageObject_TypeOffset;
  uint32_t value = Image::current().heap()[offset];
  ImageData *data = ImageData::from(value);
  if (is<ImageSmi>(data)) {
    return image_cast<ImageSmi>(data)->value();
  } else if (is<ImageForwardPointer>(data)) {
    Object *target = image_cast<ImageForwardPointer>(data)->target();
    return target->chlass()->instance_type();
  } else {
    UNREACHABLE();
    return 0;
  }
}

// -----------------------------
// --- O b j e c t   s i z e ---
// -----------------------------

uint32_t ImageObject::size_in_image() {
  uint32_t type = this->type();
  switch (type) {
    case LAMBDA_TYPE:
      return ImageLambda_Size;
    case CLASS_TYPE:
      return ImageClass_Size;
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

uint32_t ImageString::string_size_in_image() {
  return ImageString_HeaderSize + length();
}

uint32_t ImageCode::code_size_in_image() {
  return ImageCode_HeaderSize + length();
}

uint32_t ImageTuple::tuple_size_in_image() {
  return ImageTuple_HeaderSize + length();
}

}
