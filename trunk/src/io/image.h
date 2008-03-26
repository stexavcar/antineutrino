#ifndef _IO_IMAGE
#define _IO_IMAGE

#include "utils/checks.h"
#include "utils/consts.h"
#include "utils/globals.h"
#include "utils/types.h"
#include "values/values.h"

namespace neutrino {

class FLayout;
class ImageLoadInfo;

#define DECLARE_IMAGE_OBJECT_CONST(n, Type, Const)                   \
  static const uword F##Type##_##Const = n;
FOR_EACH_IMAGE_OBJECT_CONST(DECLARE_IMAGE_OBJECT_CONST)
#undef DECLARE_IMAGE_OBJECT_CONST

#define DECLARE_IMAGE_FIELD(Type, name)                              \
  inline F##Type *name(ImageLoadInfo &info, const char *location);   \
  inline void set_##name(F##Type *value)

class FData {
public:
  inline InstanceType type();
  static inline FData *from(uword value);
  static inline FData *from(Immediate *obj);
};

class FForwardPointer : public FData {
public:
  inline Object *target();
  static inline FForwardPointer *to(Object *obj);
};

class FImmediate : public FData {
public:
  inline Value *forward_pointer();
};

class FSmi : public FImmediate {
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

class FObject : public FImmediate {
public:
  inline FData *header();
  inline void set_header(FData *type);
  
  InstanceType type();
  void type_info(TypeInfo *result);
  inline void point_forward(Object *target);
  inline Object *forward_pointer();
  inline bool has_been_migrated();
  uword size_in_image();
};

class FSyntaxTree : public FObject {
  
};

class FLayout : public FObject {
public:
  inline uword instance_type();
  DECLARE_IMAGE_FIELD(Immediate, protocol);
  DECLARE_IMAGE_FIELD(Tuple, methods);
};

class FProtocol : public FObject {
public:
  DECLARE_IMAGE_FIELD(Tuple, methods);
  DECLARE_IMAGE_FIELD(Immediate, name);
  DECLARE_IMAGE_FIELD(Immediate, super);
};

class FString : public FObject {
public:
  inline uword length();
  inline void set_length(uword value);

  inline uword at(uword offset);
  inline void set(uword offset, uword value);

  static uword size_for(uword chars);
  uword string_size_in_image();
};

class FTuple : public FObject {
public:
  inline uword length();
  inline void set_length(uword value);
  
  inline FImmediate *at(uword offset);
  inline void set(uword offset, FImmediate *value);
  
  static uword size_for(uword chars);
  uword tuple_size_in_image();
};

class FCode : public FObject {
public:
  inline uword length();
  inline uword at(uword offset);
  uword code_size_in_image();
};

class FContext : public FObject {
public:
};

class FArguments : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Tuple, arguments);
  DECLARE_IMAGE_FIELD(Tuple, keyword_indices);
};

class FParameters : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Smi, position_count);
  DECLARE_IMAGE_FIELD(Tuple, parameters);
};

class FLambda : public FObject {
public:
  inline uword argc();
  DECLARE_IMAGE_FIELD(Immediate, code);
  DECLARE_IMAGE_FIELD(Immediate, literals);
  DECLARE_IMAGE_FIELD(SyntaxTree, tree);
  DECLARE_IMAGE_FIELD(Context, context);
};

class FSignature : public FObject {
public:
  DECLARE_IMAGE_FIELD(Tuple, parameters);
};

class FSelector : public FObject {
public:
  DECLARE_IMAGE_FIELD(Immediate, name);
  DECLARE_IMAGE_FIELD(Smi,   argc);
  DECLARE_IMAGE_FIELD(Tuple, keywords);
  DECLARE_IMAGE_FIELD(Immediate, is_accessor);
};

class FMethod : public FObject {
public:
  DECLARE_IMAGE_FIELD(Selector, selector);
  DECLARE_IMAGE_FIELD(Signature, signature);
  DECLARE_IMAGE_FIELD(Lambda, lambda);
};

class FDictionary : public FObject {
public:
  DECLARE_IMAGE_FIELD(Tuple, table);
};

class FStack : public FObject {
public:
};

class FTask : public FObject {
public:
  DECLARE_IMAGE_FIELD(Stack, stack);
};

class FRoot : public FObject {
public:
  inline uword index();
};

// -------------------------------
// --- S y n t a x   T r e e s ---
// -------------------------------

class FLiteralExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Immediate, value);
};

class FInvokeExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, receiver);
  DECLARE_IMAGE_FIELD(Selector, selector);
  DECLARE_IMAGE_FIELD(Immediate, arguments);
};

class FInstantiateExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, receiver);
  DECLARE_IMAGE_FIELD(String, name);
  DECLARE_IMAGE_FIELD(Arguments, arguments);
  DECLARE_IMAGE_FIELD(Tuple, terms);
};

class FRaiseExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(String, name);
  DECLARE_IMAGE_FIELD(Arguments, arguments);
};

class FLambdaExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Parameters, parameters);
  DECLARE_IMAGE_FIELD(SyntaxTree, body);
};

class FTaskExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(LambdaExpression, lambda);
};

class FDoOnExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
  DECLARE_IMAGE_FIELD(Tuple, clauses);
};

class FOnClause : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(String, name);
  DECLARE_IMAGE_FIELD(LambdaExpression, lambda);
};

class FCallExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, receiver);
  DECLARE_IMAGE_FIELD(SyntaxTree, function);
  DECLARE_IMAGE_FIELD(Arguments, arguments);
};

class FConditionalExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, condition);
  DECLARE_IMAGE_FIELD(SyntaxTree, then_part);
  DECLARE_IMAGE_FIELD(SyntaxTree, else_part);
};

class FProtocolExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(String, name);
  DECLARE_IMAGE_FIELD(Tuple, methods);
  DECLARE_IMAGE_FIELD(Immediate, super);
};

class FReturnExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
};

class FYieldExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
};

class FMethodExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Selector, selector);
  DECLARE_IMAGE_FIELD(LambdaExpression, lambda);
  DECLARE_IMAGE_FIELD(Immediate, is_static);
};

class FSequenceExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Tuple, expressions);
};

class FTupleExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Tuple, values);
};

class FSymbol : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Immediate, name);
};

class FGlobalExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(String, name);
};

class FQuoteExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
  DECLARE_IMAGE_FIELD(Tuple, unquotes);
};

class FQuoteTemplate : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
  DECLARE_IMAGE_FIELD(Tuple, unquotes);
};

class FUnquoteExpression : public FSyntaxTree {
public:
  inline uword index();
};

class FThisExpression : public FSyntaxTree {
};

class FBuiltinCall : public FSyntaxTree {
public:
  inline uword argc();
  inline uword index();
};

class FExternalCall : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Smi, argc);
  DECLARE_IMAGE_FIELD(String, name);
};

class FInterpolateExpression : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Tuple, terms);
};

class FLocalDefinition : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Symbol, symbol);
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
  DECLARE_IMAGE_FIELD(SyntaxTree, body);
};

class FAssignment : public FSyntaxTree {
public:
  DECLARE_IMAGE_FIELD(Symbol, symbol);
  DECLARE_IMAGE_FIELD(SyntaxTree, value);
};

class FForwarderDescriptor : public FObject {
public:
  DECLARE_IMAGE_FIELD(Immediate, target);
};

#define FOR_EACH_IMAGE_LOAD_STATUS(VISIT)                            \
  VISIT(TYPE_MISMATCH) VISIT(FINE)          VISIT(INVALID_IMAGE)     \
  VISIT(ROOT_COUNT)    VISIT(INVALID_MAGIC) VISIT(INVALID_VERSION)

struct ImageLoadInfo {
public:
  enum Status { 
    __first_image_load_info_status
#define MAKE_ENUM(NAME) , NAME
    FOR_EACH_IMAGE_LOAD_STATUS(MAKE_ENUM)
#undef MAKE_ENUM
  };
  ImageLoadInfo() : status(FINE) { }
  void type_mismatch(InstanceType expected, InstanceType found,
      const char *location);
  void invalid_image();
  void invalid_root_count(word expected, word found);
  void invalid_magic_number(uword found);
  void invalid_version(uword found);
  bool has_error() { return status != FINE; }
public:
  Status status;
  union {
    struct {
      InstanceType expected;
      InstanceType found;
      const char *location;
    } type_mismatch;
    struct {
      word expected;
      word found;
    } root_count;
    struct {
      uword found;
    } magic;
    struct {
      uword found;
    } version;
  } error_info;
};

template <class C>
static inline bool is(FData *val);

template <class C>
static inline C *image_cast(FData *val, ImageLoadInfo *info);

class Image {
public:
  Image(uword size, word *data);
  ~Image();
  void initialize(ImageLoadInfo &info);
  
  /**
   * Loads this image into the heap.  Returns a tuple containing the
   * roots if successful, Nothing if unsuccessful.
   */
  Data *load(ImageLoadInfo &info);
  
  static inline Image &current();
  word *heap() { return heap_; }

  class Scope {
  public:
    inline Scope(Image &image);
    inline ~Scope();
  private:
    Image *previous_;
  };

private:
  friend class ImageIterator;
  friend class Runtime;
  
  typedef void (ObjectCallback)(FObject *obj);
  static void copy_object_shallow(FObject *obj, ImageLoadInfo &load_info);
  static void fixup_shallow_object(FObject *obj, ImageLoadInfo &load_info);
  uword heap_size() { return heap_size_; }
  
  uword size_, heap_size_;
  word *data_, *heap_;
  static Image *current_;
  
  static const word kCurrentVersion    = 2;
  static const word kMagicNumber       = 0xFABACEAE;

  static const uword kMagicNumberOffset = 0;
  static const uword kVersionOffset     = 1;
  static const uword kHeapSizeOffset    = 2;
  static const uword kRootCountOffset   = 3;
  static const uword kRootsOffset       = 4;
  static const uword kHeaderSize        = 5;

};

}

#endif // _IO_IMAGE
