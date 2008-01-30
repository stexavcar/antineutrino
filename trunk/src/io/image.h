#ifndef _IO_IMAGE
#define _IO_IMAGE

#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/types.h"

namespace neutrino {

#define DECLARE_IMAGE_OBJECT_CONST(n, Type, Const)                   \
  static const uint32_t Image##Type##_##Const = n;
FOR_EACH_IMAGE_OBJECT_CONST(DECLARE_IMAGE_OBJECT_CONST)
#undef DECLARE_IMAGE_OBJECT_CONST

class ImageData {
public:
  static inline ImageData *from(uint32_t value);
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
  inline int32_t value();
  inline Smi *forward_pointer();
};

// ---------------------
// --- O b j e c t s ---
// ---------------------

class ImageObject : public ImageValue {
public:
  uint32_t type();
  void point_forward(Object *target);
  inline Object *forward_pointer();
  uint32_t size_in_image();
};

class ImageSyntaxTree : public ImageObject {
  
};

class ImageLayout : public ImageObject {
public:
  inline uint32_t instance_type();
  inline ImageTuple *methods();
  inline ImageValue *super();
  inline ImageValue *name();
};

class ImageProtocol : public ImageObject {
public:
  inline ImageTuple *methods();
  inline ImageValue *name();
};

class ImageString : public ImageObject {
public:
  inline uint32_t length();
  inline uint32_t at(uint32_t offset);
  uint32_t string_size_in_image();
};

class ImageTuple : public ImageObject {
public:
  inline uint32_t length();
  inline ImageValue *at(uint32_t offset);
  uint32_t tuple_size_in_image();
};

class ImageCode : public ImageObject {
public:
  inline uint32_t length();
  inline uint32_t at(uint32_t offset);
  uint32_t code_size_in_image();
};

class ImageContext : public ImageObject {
public:
};

class ImageLambda : public ImageObject {
public:
  inline uint32_t argc();
  inline ImageValue *code();
  inline ImageValue *literals();
  inline ImageSyntaxTree *tree();
  inline ImageContext *context();
};

class ImageSignature : public ImageObject {
public:
  inline ImageTuple *parameters();
};

class ImageMethod : public ImageObject {
public:
  inline ImageString *name();
  inline ImageSignature *signature();
  inline ImageLambda *lambda();
};

class ImageDictionary : public ImageObject {
public:
  inline ImageTuple *table();
};

class ImageStack : public ImageObject {
public:
};

class ImageTask : public ImageObject {
public:
  inline ImageStack *stack();
};

class ImageRoot : public ImageObject {
public:
  inline uint32_t index();
};

// -------------------------------
// --- S y n t a x   T r e e s ---
// -------------------------------

class ImageLiteralExpression : public ImageSyntaxTree {
public:
  inline ImageValue *value();
};

class ImageInvokeExpression : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *receiver();
  inline ImageString *name();
  inline ImageTuple *arguments();
};

class ImageInstantiateExpression : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *receiver();
  inline ImageString *name();
  inline ImageTuple *arguments();
  inline ImageTuple *terms();
};

class ImageRaiseExpression : public ImageSyntaxTree {
public:
  inline ImageString *name();
  inline ImageTuple *arguments();
};

class ImageLambdaExpression : public ImageSyntaxTree {
public:
  inline ImageTuple *params();
  inline ImageSyntaxTree *body();
};

class ImageDoOnExpression : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *value();
  inline ImageTuple *clauses();
};

class ImageOnClause : public ImageSyntaxTree {
public:
  inline ImageString *name();
  inline ImageLambdaExpression *lambda();
};

class ImageCallExpression : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *receiver();
  inline ImageSyntaxTree *function();
  inline ImageTuple *arguments();
};

class ImageConditionalExpression : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *condition();
  inline ImageSyntaxTree *then_part();
  inline ImageSyntaxTree *else_part();
};

class ImageLayoutExpression : public ImageSyntaxTree {
public:
  inline ImageString *name();
  inline ImageTuple *methods();
  inline ImageValue *super();
};

class ImageReturnExpression : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *value();
};

class ImageMethodExpression : public ImageSyntaxTree {
public:
  inline ImageString *name();
  inline ImageLambdaExpression *lambda();
};

class ImageSequenceExpression : public ImageSyntaxTree {
public:
  inline ImageTuple *expressions();
};

class ImageTupleExpression : public ImageSyntaxTree {
public:
  inline ImageTuple *values();
};

class ImageSymbol : public ImageSyntaxTree {
public:
  inline ImageValue *name();
};

class ImageGlobalExpression : public ImageSyntaxTree {
public:
  inline ImageString *name();
};

class ImageQuoteExpression : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *value();
  inline ImageTuple *unquotes();
};

class ImageQuoteTemplate : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *value();
  inline ImageTuple *unquotes();
};

class ImageUnquoteExpression : public ImageSyntaxTree {
public:
  inline uint32_t index();
};

class ImageThisExpression : public ImageSyntaxTree {
};

class ImageBuiltinCall : public ImageSyntaxTree {
public:
  inline uint32_t argc();
  inline uint32_t index();
};

class ImageInterpolateExpression : public ImageSyntaxTree {
public:
  inline ImageTuple *terms();
};

class ImageLocalDefinition : public ImageSyntaxTree {
public:
  inline ImageSymbol *symbol();
  inline ImageSyntaxTree *value();
  inline ImageSyntaxTree *body();
};

template <class C>
static inline bool is(ImageData *val);

template <class C>
static inline C *image_cast(ImageData *val);

class Image {
public:
  Image(uint32_t size, uint32_t *data);
  ~Image();
  bool initialize();
  Tuple *load();
  static inline Image &current();
  uint32_t *heap() { return heap_; }

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
  static void copy_object_shallow(ImageObject *obj);
  static void fixup_shallow_object(ImageObject *obj);
  uint32_t heap_size() { return heap_size_; }
  
  uint32_t size_, heap_size_;
  uint32_t *data_, *heap_;
  static Image *current_;
  
  static const uint32_t kMagicNumber = 0xFABACEAE;
  static const uint32_t kMagicNumberOffset = 0;
  static const uint32_t kHeapSizeOffset    = kMagicNumberOffset + 1;
  static const uint32_t kRootsOffset       = kHeapSizeOffset + 1;
  static const uint32_t kHeaderSize        = kRootsOffset + 1;

};

class ImageIterator {
public:
  inline ImageIterator(Image &image);
  inline bool has_next();
  inline ImageObject *next();
  inline void reset();
private:
  uint32_t cursor() { return cursor_; }
  uint32_t limit() { return limit_; }
  uint32_t cursor_;
  uint32_t limit_;
};

}

#endif // _IO_IMAGE
