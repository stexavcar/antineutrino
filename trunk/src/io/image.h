#ifndef _IO_IMAGE
#define _IO_IMAGE

#include "utils/checks.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/types.h"
#include "values/values.h"

namespace neutrino {

class ImageLayout;
class ImageLoadInfo;

#define DECLARE_IMAGE_OBJECT_CONST(n, Type, Const)                   \
  static const uword Image##Type##_##Const = n;
FOR_EACH_IMAGE_OBJECT_CONST(DECLARE_IMAGE_OBJECT_CONST)
#undef DECLARE_IMAGE_OBJECT_CONST

#define DECLARE_IMAGE_FIELD(Type, name)                              \
  inline Image##Type *name(ImageLoadInfo &info)

class ImageData {
public:
  inline InstanceType type();
  static inline ImageData *from(uword value);
};

class ImageForwardPointer : public ImageData {
public:
  inline Object *target();
  static inline ImageForwardPointer *to(Object *obj);
};

class ImageValue : public ImageData {
public:
  Value *forward_pointer();
};

class ImageSmi : public ImageValue {
public:
  inline word value();
  inline Smi *forward_pointer();
};

// ---------------------
// --- O b j e c t s ---
// ---------------------

class TypeInfo {
public:
  TypeInfo() : type(__illegal_instance_type), layout(0) { }
  bool has_layout() { return layout != 0; }
  InstanceType type;
  Layout *layout;
};

class ImageObject : public ImageValue {
public:
  InstanceType type();
  void type_info(TypeInfo *result);
  void point_forward(Object *target);
  inline Object *forward_pointer();
  inline ImageData *header();
  inline bool has_been_migrated();
  uword size_in_image();
};

class ImageSyntaxTree : public ImageObject {
  
};

class ImageLayout : public ImageObject {
public:
  inline uword instance_type();
  DECLARE_IMAGE_FIELD(Value, protocol);
  DECLARE_IMAGE_FIELD(Tuple, methods);
};

class ImageProtocol : public ImageObject {
public:
  DECLARE_IMAGE_FIELD(Tuple, methods);
  DECLARE_IMAGE_FIELD(Value, name);
  DECLARE_IMAGE_FIELD(Value, super);
};

class ImageString : public ImageObject {
public:
  inline uword length();
  inline uword at(uword offset);
  uword string_size_in_image();
};

class ImageTuple : public ImageObject {
public:
  inline uword length();
  inline ImageValue *at(uword offset);
  uword tuple_size_in_image();
};

class ImageCode : public ImageObject {
public:
  inline uword length();
  inline uword at(uword offset);
  uword code_size_in_image();
};

class ImageContext : public ImageObject {
public:
};

class ImageLambda : public ImageObject {
public:
  inline uword argc();
  DECLARE_IMAGE_FIELD(Value, code);
  DECLARE_IMAGE_FIELD(Value, literals);
  DECLARE_IMAGE_FIELD(SyntaxTree, tree);
  DECLARE_IMAGE_FIELD(Context, context);
};

class ImageSignature : public ImageObject {
public:
  DECLARE_IMAGE_FIELD(Tuple, parameters);
};

class ImageSelector : public ImageObject {
public:
  DECLARE_IMAGE_FIELD(Value, name);
  DECLARE_IMAGE_FIELD(Smi, argc);
};

class ImageMethod : public ImageObject {
public:
  DECLARE_IMAGE_FIELD(Selector, selector);
  DECLARE_IMAGE_FIELD(Signature, signature);
  DECLARE_IMAGE_FIELD(Lambda, lambda);
};

class ImageDictionary : public ImageObject {
public:
  DECLARE_IMAGE_FIELD(Tuple, table);
};

class ImageStack : public ImageObject {
public:
};

class ImageTask : public ImageObject {
public:
  DECLARE_IMAGE_FIELD(Stack, stack);
};

class ImageRoot : public ImageObject {
public:
  inline uword index();
};

// -------------------------------
// --- S y n t a x   T r e e s ---
// -------------------------------

class ImageLiteralExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Value, value);
};

class ImageArguments : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Tuple, arguments);
  DECLARE_IMAGE_FIELD(Tuple, keywords);
};

class ImageInvokeExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, receiver);
  DECLARE_IMAGE_FIELD(Selector, selector);
  DECLARE_IMAGE_FIELD(Arguments, arguments);
};

class ImageInstantiateExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, receiver);
  DECLARE_IMAGE_FIELD(String, name);
  DECLARE_IMAGE_FIELD(Arguments, arguments);
  DECLARE_IMAGE_FIELD(Tuple, terms);
};

class ImageRaiseExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(String, name);
  DECLARE_IMAGE_FIELD(Arguments, arguments);
};

class ImageLambdaExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Tuple, params);
  DECLARE_IMAGE_FIELD(SyntaxTree, body);
};

class ImageDoOnExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
  DECLARE_IMAGE_FIELD(Tuple, clauses);
};

class ImageOnClause : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(String, name);
  DECLARE_IMAGE_FIELD(LambdaExpression, lambda);
};

class ImageCallExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, receiver);
  DECLARE_IMAGE_FIELD(SyntaxTree, function);
  DECLARE_IMAGE_FIELD(Arguments, arguments);
};

class ImageConditionalExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, condition);
  DECLARE_IMAGE_FIELD(SyntaxTree, then_part);
  DECLARE_IMAGE_FIELD(SyntaxTree, else_part);
};

class ImageProtocolExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(String, name);
  DECLARE_IMAGE_FIELD(Tuple, methods);
  DECLARE_IMAGE_FIELD(Value, super);
};

class ImageReturnExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
};

class ImageMethodExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Selector, selector);
  DECLARE_IMAGE_FIELD(LambdaExpression, lambda);
  DECLARE_IMAGE_FIELD(Value, is_static);
};

class ImageSequenceExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Tuple, expressions);
};

class ImageTupleExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Tuple, values);
};

class ImageSymbol : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Value, name);
};

class ImageGlobalExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(String, name);
};

class ImageQuoteExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
  DECLARE_IMAGE_FIELD(Tuple, unquotes);
};

class ImageQuoteTemplate : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
  DECLARE_IMAGE_FIELD(Tuple, unquotes);
};

class ImageUnquoteExpression : public ImageSyntaxTree {
public:
  inline uword index();
};

class ImageThisExpression : public ImageSyntaxTree {
};

class ImageBuiltinCall : public ImageSyntaxTree {
public:
  inline uword argc();
  inline uword index();
};

class ImageExternalCall : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Smi, argc);
  DECLARE_IMAGE_FIELD(String, name);
};

class ImageInterpolateExpression : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Tuple, terms);
};

class ImageLocalDefinition : public ImageSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Symbol, symbol);
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
  DECLARE_IMAGE_FIELD(SyntaxTree, body);
};

class ImageForwarderDescriptor : public ImageObject {
public:
  DECLARE_IMAGE_FIELD(Value, target);
};

#define FOR_EACH_IMAGE_LOAD_STATUS(VISIT)                            \
  VISIT(TYPE_MISMATCH) VISIT(FINE)          VISIT(INVALID_IMAGE)     \
  VISIT(ROOT_COUNT)

struct ImageLoadInfo {
public:
  enum Status { 
    __first_image_load_info_status
#define MAKE_ENUM(NAME) , NAME
    FOR_EACH_IMAGE_LOAD_STATUS(MAKE_ENUM)
#undef MAKE_ENUM
  };
  ImageLoadInfo() : status(FINE) { }
  void type_mismatch(InstanceType expected, InstanceType found);
  void invalid_image();
  void invalid_root_count(word expected, word found);
  bool has_error() { return status != FINE; }
public:
  Status status;
  union {
    struct {
      InstanceType expected;
      InstanceType found;
    } type_mismatch;
    struct {
      word expected;
      word found;
    } root_count;
  } error_info;
};

template <class C>
static inline bool is(ImageData *val);

template <class C>
static inline C *image_cast(ImageData *val, ImageLoadInfo *info);

class Image {
public:
  Image(uword size, uword *data);
  ~Image();
  void initialize(ImageLoadInfo &info);
  
  /**
   * Loads this image into the heap.  Returns a tuple containing the
   * roots if successful, Nothing if unsuccessful.
   */
  Data *load(ImageLoadInfo &info);
  
  static inline Image &current();
  uword *heap() { return heap_; }

  class Scope {
  public:
    Scope(Image &image);
    ~Scope();
  private:
    Image *previous_;
  };

private:
  friend class ImageIterator;
  typedef void (ObjectCallback)(ImageObject *obj);
  static void copy_object_shallow(ImageObject *obj, ImageLoadInfo &load_info);
  static void fixup_shallow_object(ImageObject *obj, ImageLoadInfo &load_info);
  uword heap_size() { return heap_size_; }
  
  uword size_, heap_size_;
  uword *data_, *heap_;
  static Image *current_;
  
  static const uword kMagicNumber = 0xFABACEAE;
  static const uword kMagicNumberOffset = 0;
  static const uword kHeapSizeOffset    = kMagicNumberOffset + 1;
  static const uword kRootCountOffset   = kHeapSizeOffset + 1;
  static const uword kRootsOffset       = kRootCountOffset + 1;
  static const uword kHeaderSize        = kRootsOffset + 1;

};

class ImageIterator {
public:
  inline ImageIterator(Image &image);
  inline bool has_next();
  inline ImageObject *next();
  inline void reset();
private:
  uword cursor() { return cursor_; }
  uword limit() { return limit_; }
  uword cursor_;
  uword limit_;
};

}

#endif // _IO_IMAGE
