#ifndef _VALUES_METHOD
#define _VALUES_METHOD

namespace neutrino {


class MethodLookup {
public:
  
  /**
   * Create a new method lookup object.  The min score parameter is
   * the minimal score allowed for a method.  This is used when
   * selecting super methods.
   */
  inline MethodLookup(uword min_score);

  void lookup_method(Layout *layout, Selector *selector);
  
  Data *method() { return method_; }
  
  bool is_ambiguous() { return is_ambiguous_; }
  
  /**
   * Returns the given method's score when applied to the specified
   * receiver.
   */
  static inline uword get_method_score(Signature *signature,
      Layout *receiver);

  static const uword kNoMatch = TypeConsts<uword>::kMax;

private:
  
  /**
   * Scores the given method as applied to the specified receiver type
   * and applies the score to this method lookup.
   */
  void score_method(Method *method, Layout *reciever);
  
  static inline uword get_distance(Layout *layout, Protocol *target);
  void lookup_in_dictionary(Tuple *methods, Selector *name, Layout *receiver);

  uword score_;
  uword min_score_;
  Data *method_;
  bool is_ambiguous_;
};


// -------------------
// --- M e t h o d ---
// -------------------

#define eMethodFields(VISIT, arg)                            \
  VISIT(Selector,  selector,  Selector,  arg)                        \
  VISIT(Signature, signature, Signature, arg)                        \
  VISIT(Lambda,    lambda,    Lambda,    arg)

class Method : public Object {
public:
  eMethodFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kSelectorOffset  = Object::kHeaderSize;
  static const uword kSignatureOffset = kSelectorOffset + kPointerSize;
  static const uword kLambdaOffset    = kSignatureOffset + kPointerSize;
  static const uword kSize            = kLambdaOffset + kPointerSize;
};

template <> class ref_traits<Method> : public ref_traits<Object> {
public:
  eMethodFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Method);


// -------------------------
// --- S i g n a t u r e ---
// -------------------------

#define eSignatureFields(VISIT, arg)                         \
  VISIT(Tuple, parameters, Parameters, arg)

class Signature : public Object {
public:
  eSignatureFields(DECLARE_OBJECT_FIELD, 0)

  static const uword kParametersOffset = Object::kHeaderSize;
  static const uword kSize             = kParametersOffset + kPointerSize;
};

template <> class ref_traits<Signature> : public ref_traits<Object> {
public:
  eSignatureFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Signature);


// -----------------------
// --- S e l e c t o r ---
// -----------------------

#define eSelectorFields(VISIT, arg)                          \
  VISIT(Immediate, name,        Name,       arg)                     \
  VISIT(Smi,       argc,        Argc,       arg)                     \
  VISIT(Tuple,     keywords,    Keywords,   arg)                     \
  VISIT(Bool,      is_accessor, IsAccessor, arg)

class Selector : public Object {
public:
  eSelectorFields(DECLARE_OBJECT_FIELD, 0)

  bool selector_equals(Selector *that);

  static const uword kNameOffset       = Object::kHeaderSize;
  static const uword kArgcOffset       = kNameOffset + kPointerSize;
  static const uword kKeywordsOffset   = kArgcOffset + kPointerSize;
  static const uword kIsAccessorOffset = kKeywordsOffset + kPointerSize;
  static const uword kSize             = kIsAccessorOffset + kPointerSize;
};

template <> class ref_traits<Selector> : public ref_traits<Object> {
public:
  eSelectorFields(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Selector);


} // neutrino

#endif // _VALUES_METHOD
