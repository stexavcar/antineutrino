#ifndef _IO_IMAGE
#define _IO_IMAGE

#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/types.h"

namespace neutrino {

#define DECLARE_IMAGE_OBJECT_CONST(n, Type, Const)                   \
  static const uword Image##Type##_##Const = n;
FOR_EACH_IMAGE_OBJECT_CONST(DECLARE_IMAGE_OBJECT_CONST)
#undef DECLARE_IMAGE_OBJECT_CONST

class ImageData {
public:
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

class ImageObject : public ImageValue {
public:
  uword type();
  void point_forward(Object *target);
  inline Object *forward_pointer();
  uword size_in_image();
};

class ImageSyntaxTree : public ImageObject {
  
};

class ImageLayout : public ImageObject {
public:
  inline uword instance_type();
  inline ImageValue *protocol();
  inline ImageTuple *methods();
};

class ImageProtocol : public ImageObject {
public:
  inline ImageTuple *methods();
  inline ImageValue *name();
  inline ImageValue *super();
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
  inline uword index();
};

// -------------------------------
// --- S y n t a x   T r e e s ---
// -------------------------------

class ImageLiteralExpression : public ImageSyntaxTree {
public:
  inline ImageValue *value();
};

class ImageArguments : public ImageSyntaxTree {
public:
  inline ImageTuple *arguments();
  inline ImageTuple *keywords();
};

class ImageInvokeExpression : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *receiver();
  inline ImageString *name();
  inline ImageArguments *arguments();
};

class ImageInstantiateExpression : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *receiver();
  inline ImageString *name();
  inline ImageArguments *arguments();
  inline ImageTuple *terms();
};

class ImageRaiseExpression : public ImageSyntaxTree {
public:
  inline ImageString *name();
  inline ImageArguments *arguments();
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
  inline ImageArguments *arguments();
};

class ImageConditionalExpression : public ImageSyntaxTree {
public:
  inline ImageSyntaxTree *condition();
  inline ImageSyntaxTree *then_part();
  inline ImageSyntaxTree *else_part();
};

class ImageProtocolExpression : public ImageSyntaxTree {
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
  inline ImageSmi *argc();
  inline ImageString *name();
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

class ImageForwarderDescriptor : public ImageObject {
public:
  inline ImageValue *target();
};

template <class C>
static inline bool is(ImageData *val);

template <class C>
static inline C *image_cast(ImageData *val);

class Image {
public:
  Image(uword size, uword *data);
  ~Image();
  bool initialize();
  Tuple *load();
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
  static void copy_object_shallow(ImageObject *obj);
  static void fixup_shallow_object(ImageObject *obj);
  uword heap_size() { return heap_size_; }
  
  uword size_, heap_size_;
  uword *data_, *heap_;
  static Image *current_;
  
  static const uword kMagicNumber = 0xFABACEAE;
  static const uword kMagicNumberOffset = 0;
  static const uword kHeapSizeOffset    = kMagicNumberOffset + 1;
  static const uword kRootsOffset       = kHeapSizeOffset + 1;
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
