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
  DisallowGarbageCollection dgc;
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
      Class *chlass = cast<Class>(heap.new_empty_class(instance_type));
      obj->point_forward(chlass);
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
    case DICTIONARY_TYPE: {
      ImageDictionary *img = image_cast<ImageDictionary>(obj);
      Dictionary *dict = cast<Dictionary>(img->forward_pointer());
      dict->set_table(cast<Tuple>(img->table()->forward_pointer()));
      break;
    }
    case TUPLE_TYPE: {
      ImageTuple *img = image_cast<ImageTuple>(obj);
      Tuple *tuple = cast<Tuple>(img->forward_pointer());
      uint32_t length = tuple->length();
      for (uint32_t i = 0; i < length; i++)
        tuple->at(i) = img->at(i)->forward_pointer();
      break;
    }
    case LAMBDA_TYPE: {
      ImageLambda *img = image_cast<ImageLambda>(obj);
      Lambda *lambda = cast<Lambda>(img->forward_pointer());
      lambda->set_code(img->code()->forward_pointer());
      lambda->set_literals(img->literals()->forward_pointer());
      lambda->set_tree(cast<LambdaExpression>(img->tree()->forward_pointer()));
      lambda->set_outers(Runtime::current().heap().roots().empty_tuple());
      break;
    }
    case METHOD_TYPE: {
      ImageMethod *img = image_cast<ImageMethod>(obj);
      Method *method = cast<Method>(img->forward_pointer());
      method->set_name(cast<String>(img->name()->forward_pointer()));
      method->set_lambda(cast<Lambda>(img->lambda()->forward_pointer()));
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
    case LITERAL_EXPRESSION_TYPE: {
      ImageLiteralExpression *img = image_cast<ImageLiteralExpression>(obj);
      LiteralExpression *literal = cast<LiteralExpression>(img->forward_pointer());
      literal->set_value(img->value()->forward_pointer());
      break;
    }
    case INVOKE_EXPRESSION_TYPE: {
      ImageInvokeExpression *img = image_cast<ImageInvokeExpression>(obj);
      InvokeExpression *invoke = cast<InvokeExpression>(img->forward_pointer());
      invoke->set_receiver(cast<SyntaxTree>(img->receiver()->forward_pointer()));
      invoke->set_name(cast<String>(img->name()->forward_pointer()));
      invoke->set_arguments(cast<Tuple>(img->arguments()->forward_pointer()));
      break;
    }
    case CALL_EXPRESSION_TYPE: {
      ImageCallExpression *img = image_cast<ImageCallExpression>(obj);
      CallExpression *invoke = cast<CallExpression>(img->forward_pointer());
      invoke->set_receiver(cast<SyntaxTree>(img->receiver()->forward_pointer()));
      invoke->set_function(cast<SyntaxTree>(img->function()->forward_pointer()));
      invoke->set_arguments(cast<Tuple>(img->arguments()->forward_pointer()));
      break;
    }
    case CONDITIONAL_EXPRESSION_TYPE: {
      ImageConditionalExpression *img = image_cast<ImageConditionalExpression>(obj);
      ConditionalExpression *expr = cast<ConditionalExpression>(img->forward_pointer());
      expr->set_condition(cast<SyntaxTree>(img->condition()->forward_pointer()));
      expr->set_then_part(cast<SyntaxTree>(img->then_part()->forward_pointer()));
      expr->set_else_part(cast<SyntaxTree>(img->else_part()->forward_pointer()));
      break;
    }
    case CLASS_EXPRESSION_TYPE: {
      ImageClassExpression *img = image_cast<ImageClassExpression>(obj);
      ClassExpression *expr = cast<ClassExpression>(img->forward_pointer());
      expr->set_name(cast<String>(img->name()->forward_pointer()));
      expr->set_methods(cast<Tuple>(img->methods()->forward_pointer()));
      expr->set_super(img->super()->forward_pointer());
      break;
    }
    case RETURN_EXPRESSION_TYPE: {
      ImageReturnExpression *img = image_cast<ImageReturnExpression>(obj);
      ReturnExpression *expr = cast<ReturnExpression>(img->forward_pointer());
      expr->set_value(cast<SyntaxTree>(img->value()->forward_pointer()));
      break;
    }
    case METHOD_EXPRESSION_TYPE: {
      ImageMethodExpression *img = image_cast<ImageMethodExpression>(obj);
      MethodExpression *expr = cast<MethodExpression>(img->forward_pointer());
      expr->set_name(cast<String>(img->name()->forward_pointer()));
      expr->set_lambda(cast<LambdaExpression>(img->lambda()->forward_pointer()));
      break;
    }
    case LAMBDA_EXPRESSION_TYPE: {
      ImageLambdaExpression *img = image_cast<ImageLambdaExpression>(obj);
      LambdaExpression *expr = cast<LambdaExpression>(img->forward_pointer());
      expr->set_params(cast<Tuple>(img->params()->forward_pointer()));
      expr->set_body(cast<SyntaxTree>(img->body()->forward_pointer()));
      break;
    }
    case SEQUENCE_EXPRESSION_TYPE: {
      ImageSequenceExpression *img = image_cast<ImageSequenceExpression>(obj);
      SequenceExpression *expr = cast<SequenceExpression>(img->forward_pointer());
      expr->set_expressions(cast<Tuple>(img->expressions()->forward_pointer()));
      break;
    }
    case TUPLE_EXPRESSION_TYPE: {
      ImageTupleExpression *img = image_cast<ImageTupleExpression>(obj);
      TupleExpression *expr = cast<TupleExpression>(img->forward_pointer());
      expr->set_values(cast<Tuple>(img->values()->forward_pointer()));
      break;
    }
    case SYMBOL_TYPE: {
      ImageSymbol *img = image_cast<ImageSymbol>(obj);
      Symbol *sym = cast<Symbol>(img->forward_pointer());
      sym->set_name(img->name()->forward_pointer());
      break;
    }
    case QUOTE_EXPRESSION_TYPE: {
      ImageQuoteExpression *img = image_cast<ImageQuoteExpression>(obj);
      QuoteExpression *expr = cast<QuoteExpression>(img->forward_pointer());
      expr->set_value(cast<SyntaxTree>(img->value()->forward_pointer()));
      break;
    }
    case GLOBAL_EXPRESSION_TYPE: {
      ImageGlobalExpression *img = image_cast<ImageGlobalExpression>(obj);
      GlobalExpression *expr = cast<GlobalExpression>(img->forward_pointer());
      expr->set_name(cast<String>(img->name()->forward_pointer()));
      break;
    }
    case INTERPOLATE_EXPRESSION_TYPE: {
      ImageInterpolateExpression *img = image_cast<ImageInterpolateExpression>(obj);
      InterpolateExpression *expr = cast<InterpolateExpression>(img->forward_pointer());
      expr->set_terms(cast<Tuple>(img->terms()->forward_pointer()));
      break;
    }
    case LOCAL_DEFINITION_TYPE: {
      ImageLocalDefinition *img = image_cast<ImageLocalDefinition>(obj);
      LocalDefinition *expr = cast<LocalDefinition>(img->forward_pointer());
      expr->set_symbol(cast<Symbol>(img->symbol()->forward_pointer()));
      expr->set_value(cast<SyntaxTree>(img->value()->forward_pointer()));
      expr->set_body(cast<SyntaxTree>(img->body()->forward_pointer()));
      break;
    }
    case BUILTIN_CALL_TYPE: {
      ImageBuiltinCall *img = image_cast<ImageBuiltinCall>(obj);
      BuiltinCall *expr = cast<BuiltinCall>(img->forward_pointer());
      expr->set_argc(img->argc());
      expr->set_index(img->index());
      break;
    }
    case STRING_TYPE: case CODE_TYPE: case SINGLETON_TYPE:
    case THIS_EXPRESSION_TYPE:
      // Nothing to fix
      break;
    default:
      UNHANDLED(InstanceType, type);
      break;
  }
  // IF_DEBUG(if (obj->type() != SINGLETON_TYPE) obj->forward_pointer()->validate());
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
    case DICTIONARY_TYPE:
      return ImageDictionary_Size;
    case LAMBDA_TYPE:
      return ImageLambda_Size;
    case CLASS_TYPE:
      return ImageClass_Size;
    case METHOD_TYPE:
      return ImageMethod_Size;
    case LITERAL_EXPRESSION_TYPE:
      return ImageLiteralExpression_Size;
    case INVOKE_EXPRESSION_TYPE:
      return ImageInvokeExpression_Size;
    case CALL_EXPRESSION_TYPE:
      return ImageCallExpression_Size;
    case CONDITIONAL_EXPRESSION_TYPE:
      return ImageConditionalExpression_Size;
    case CLASS_EXPRESSION_TYPE:
      return ImageClassExpression_Size;
    case RETURN_EXPRESSION_TYPE:
      return ImageReturnExpression_Size;
    case METHOD_EXPRESSION_TYPE:
      return ImageMethodExpression_Size;
    case SEQUENCE_EXPRESSION_TYPE:
      return ImageSequenceExpression_Size;
    case TUPLE_EXPRESSION_TYPE:
      return ImageTupleExpression_Size;
    case GLOBAL_EXPRESSION_TYPE:
      return ImageGlobalExpression_Size;
    case SYMBOL_TYPE:
      return ImageSymbol_Size;
    case THIS_EXPRESSION_TYPE:
      return ImageThisExpression_Size;
    case QUOTE_EXPRESSION_TYPE:
      return ImageQuoteExpression_Size;
    case LAMBDA_EXPRESSION_TYPE:
      return ImageLambdaExpression_Size;
    case SINGLETON_TYPE:
      return ImageRoot_Size;
    case BUILTIN_CALL_TYPE:
      return ImageBuiltinCall_Size;
    case INTERPOLATE_EXPRESSION_TYPE:
      return ImageInterpolateExpression_Size;
    case LOCAL_DEFINITION_TYPE:
      return ImageLocalDefinition_Size;
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
